#include <simgrid/s4u/Engine.hpp>
#include <simgrid/s4u/Mailbox.hpp>
#include <tuple>
#include <unordered_map>
#include <variant>
#include <xbt/log.h>

#include "asynchronous_aggregator.hpp"
#include "../../network_managers/nm.hpp"
#include "../../../protocol.hpp"
#include "../../../utils/utils.hpp"

using namespace std;

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
    this->number_client_training = 
        this->get_network_manager()->handle_registration_requests();

    // Send the global model to everyone
    this->broadcast_global_model();

    auto sim_time = simgrid::s4u::Engine::get_instance()->get_clock();

    while (simgrid::s4u::Engine::get_instance()->get_clock() < sim_time + Constants::DURATION_TRAINING_PHASE)
    {
        auto tupple = this->wait_local_model();
        node_name src = std::get<0>(tupple); 
        node_name original_src = std::get<0>(tupple); 

        this->aggregate(1);
        this->send_global_model(src, original_src);
    }

    this->send_kills();

    this->print_end_report();
}


void AsynchronousAggregator::broadcast_global_model()
{
    auto nm = this->get_network_manager();

    auto p = make_shared<Packet>(Packet(
        this->get_network_manager()->get_my_node_name(), "BROADCAST",
        Packet::SendGlobalModel(
            this->number_local_epochs
        )
    ));

    nm->broadcast(p, Filters::trainers_and_aggregators);
}

/* Sends the global model to every start_nodes */
void AsynchronousAggregator::send_global_model(node_name dst, node_name final_dst)
{
    auto nm = this->get_network_manager();

    auto p = make_shared<Packet>(Packet(
        this->get_network_manager()->get_my_node_name(), final_dst,
        Packet::SendGlobalModel(
            this->number_local_epochs
        )
    ));

    nm->send(p, dst);
}

std::tuple<node_name, node_name> AsynchronousAggregator::wait_local_model()
{
    std::unique_ptr<Packet> p;
    node_name res;
    auto nm = this->get_network_manager();
    bool cond = true;

    // Note that here we don't check that the local models come from different trainers
    while (cond) {
        p = nm->get_packet();

        if (auto *send_local = get_if<Packet::SendLocalModel>(&p->op))
        {
            res = p->src;
            cond = false;
        }
    }    

    return std::tuple<node_name, node_name>(p->src, p->original_src);
}

void AsynchronousAggregator::send_kills()
{
    auto nm = this->get_network_manager();

    auto p = make_shared<Packet>(Packet(
        this->get_network_manager()->get_my_node_name(), "BROADCAST",
        Packet::KillTrainer()
    ));

    nm->broadcast(p, Filters::trainers_and_aggregators);
}
