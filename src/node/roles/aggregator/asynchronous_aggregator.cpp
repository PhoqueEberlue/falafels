#include <simgrid/s4u/Engine.hpp>
#include <simgrid/s4u/Mailbox.hpp>
#include <unordered_map>
#include <xbt/log.h>

#include "asynchronous_aggregator.hpp"
#include "../../network_managers/nm.hpp"
#include "../../../protocol.hpp"
#include "../../../utils/utils.hpp"

XBT_LOG_NEW_DEFAULT_CATEGORY(s4u_asynchronous_aggregator, "Messages specific for this example");

AsynchronousAggregator::AsynchronousAggregator(std::unordered_map<std::string, std::string> *args)
{
    for (auto &[key, value]: *args)
    {
        switch (str2int(key.c_str()))
        {
            case str2int("proportion_threshold"):
                XBT_INFO("proportion_threshold=%s", value.c_str());
                this->proportion_threshold = std::stof(value);
                break;
        }
    }

    delete args;
}

void AsynchronousAggregator::run()
{
    // At first send the global model to everyone
    this->broadcast_global_model();

    while (simgrid::s4u::Engine::get_instance()->get_clock() < 2)
    {
        node_name src = this->wait_local_model();
        this->aggregate(1);
        this->send_global_model(src);
    } 

    this->send_kills();

    simgrid::s4u::this_actor::exit();
}

void AsynchronousAggregator::broadcast_global_model()
{
    auto nm = this->get_network_manager();
    auto args = new std::unordered_map<std::string, std::string>();
    args->insert({ "number_local_epochs", std::to_string(this->number_local_epochs) });
    Packet *p = new Packet(Packet::Operation::SEND_GLOBAL_MODEL, nm->get_my_node_name(), args);

    XBT_INFO("%s ---%s--> *TRAINERS", p->src.c_str(), p->op_string.c_str());
    nm->broadcast(p, Filters::trainers);
}

/* Sends the global model to every start_nodes */
void AsynchronousAggregator::send_global_model(node_name name)
{
    auto nm = this->get_network_manager();
    auto args = new std::unordered_map<std::string, std::string>();
    args->insert({ "number_local_epochs", std::to_string(this->number_local_epochs) });
    Packet *p = new Packet(Packet::Operation::SEND_GLOBAL_MODEL, nm->get_my_node_name(), args);

    XBT_INFO("%s ---%s--> %s", p->src.c_str(), p->op_string.c_str(), name.c_str());

    nm->send(p, name);
}

node_name AsynchronousAggregator::wait_local_model()
{
    Packet *p;
    node_name res;
    auto nm = this->get_network_manager();
    bool cond = true;

    // Note that here we don't check that the local models come from different trainers
    while (cond) {
        p = nm->get();
        XBT_INFO("%s <--%s--- %s", nm->get_my_node_name().c_str(), p->op_string.c_str(), p->src.c_str());

        if (p->op == Packet::Operation::SEND_LOCAL_MODEL)
        {
            res = p->src;
            cond = false;
        }
        p->decr_ref_count();
    }    

    return res;
}

void AsynchronousAggregator::send_kills()
{
    auto nm = this->get_network_manager();
    Packet *p;

    p = new Packet(Packet::Operation::KILL_TRAINER, nm->get_my_node_name());

    XBT_INFO("%s ---%s--> *TRAINERS", p->src.c_str(), p->op_string.c_str());

    nm->broadcast(p, Filters::trainers);
}
