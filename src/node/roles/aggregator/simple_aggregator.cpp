#include <cstdint>
#include <memory>
#include <simgrid/s4u/Engine.hpp>
#include <variant>
#include <xbt/log.h>

#include "simple_aggregator.hpp"
#include "../../../protocol.hpp"

using namespace std;

XBT_LOG_NEW_DEFAULT_CATEGORY(s4u_simple_aggregator, "Messages specific for this example");

SimpleAggregator::SimpleAggregator(std::unordered_map<std::string, std::string> *args)
{
    // No arguments yet
    delete args;
}

static bool trainer_filter(NodeInfo *node_info)
{
    return node_info->role == NodeRole::Trainer;
}

void SimpleAggregator::run()
{
    // Wait for the trainers to register.
    this->number_client_training = 
        this->get_network_manager()->handle_registration_requests();

    auto current_sim_time = simgrid::s4u::Engine::get_instance()->get_clock();

    while (simgrid::s4u::Engine::get_instance()->get_clock() < current_sim_time + Constants::DURATION_TRAINING_PHASE)
    {
        this->send_global_model();
        uint64_t number_local_models = this->wait_local_models();
        this->aggregate(number_local_models);
    } 

    this->send_kills();

    this->print_end_report();
    simgrid::s4u::this_actor::exit();
}


/* Sends the global model to every start_nodes */
void SimpleAggregator::send_global_model()
{
    auto p = make_shared<Packet>(Packet(
        this->get_network_manager()->get_my_node_name(), "BROADCAST",
        Packet::SendGlobalModel(
            this->number_local_epochs
        )
    ));

    this->get_network_manager()->broadcast(p, Filters::trainers);
}

uint64_t SimpleAggregator::wait_local_models()
{
    uint64_t number_local_models = 0;
    auto nm                      = this->get_network_manager();
    std::unique_ptr<Packet> p;

    // While we don't have enough local models
    while (number_local_models < this->number_client_training)
    {
        XBT_INFO("nb local models: %lu", number_local_models);
        p = nm->get_packet();

        // Note that here we don't check that the local models come from different trainers
        if (get_if<Packet::SendLocalModel>(&p->op))
        {
            number_local_models += 1;
        }
    }
    
    XBT_INFO("Retrieved %lu local models out of %i", number_local_models, this->number_client_training);

    return number_local_models;
}

void SimpleAggregator::send_kills()
{
    auto p = make_shared<Packet>(Packet(
        this->get_network_manager()->get_my_node_name(), "BROADCAST",
        Packet::KillTrainer()
    ));

    this->get_network_manager()->broadcast(p, Filters::trainers);
}
