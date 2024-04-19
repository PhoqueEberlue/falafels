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
    // Wait for the trainers to register.
    this->get_network_manager()->handle_registration_requests();

    // Send the global model to everyone
    this->broadcast_global_model();

    auto current_sim_time = simgrid::s4u::Engine::get_instance()->get_clock();

    while (simgrid::s4u::Engine::get_instance()->get_clock() < current_sim_time + Constants::DURATION_TRAINING_PHASE)
    {
        node_name src = this->wait_local_model();
        this->aggregate(1);
        this->send_global_model(src);
    }

    this->send_kills();

    this->print_end_report();
    simgrid::s4u::this_actor::exit();
}


void AsynchronousAggregator::broadcast_global_model()
{
    auto nm = this->get_network_manager();

    Packet *p = new Packet(
        this->get_network_manager()->get_my_node_name(), "BROADCAST",
        Packet::Operation::SEND_GLOBAL_MODEL, 
        new Packet::Data { .number_local_epochs=this->number_local_epochs }
    );

    auto nb_sent = nm->broadcast(p, Filters::trainers);

    this->number_client_training = nb_sent;
}

/* Sends the global model to every start_nodes */
void AsynchronousAggregator::send_global_model(node_name dst)
{
    auto nm = this->get_network_manager();

    Packet *p = new Packet(
        this->get_network_manager()->get_my_node_name(), dst,
        Packet::Operation::SEND_GLOBAL_MODEL, 
        new Packet::Data { .number_local_epochs=this->number_local_epochs }
    );

    nm->send(p, dst);
}

node_name AsynchronousAggregator::wait_local_model()
{
    std::unique_ptr<Packet> p;
    node_name res;
    auto nm = this->get_network_manager();
    bool cond = true;

    // Note that here we don't check that the local models come from different trainers
    while (cond) {
        p = nm->get();

        if (p->op == Packet::Operation::SEND_LOCAL_MODEL)
        {
            res = p->src;
            cond = false;
        }
    }    

    return res;
}

void AsynchronousAggregator::send_kills()
{
    auto nm = this->get_network_manager();
    Packet *p;

    p = new Packet(
        this->get_network_manager()->get_my_node_name(), "BROADCAST",
        Packet::Operation::KILL_TRAINER
    );

    nm->broadcast(p, Filters::trainers);
}
