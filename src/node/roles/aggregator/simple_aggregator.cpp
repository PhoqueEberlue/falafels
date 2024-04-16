#include <cstdint>
#include <memory>
#include <simgrid/s4u/Engine.hpp>
#include <xbt/log.h>

#include "simple_aggregator.hpp"
#include "../../../protocol.hpp"

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
    this->get_network_manager()->handle_registration_requests();

    auto current_sim_time = simgrid::s4u::Engine::get_instance()->get_clock();

    while (simgrid::s4u::Engine::get_instance()->get_clock() < current_sim_time + Constants::DURATION_TRAINING_PHASE)
    {
        this->send_global_model();
        uint64_t number_local_models = this->wait_local_models();
        this->aggregate(number_local_models);
    } 

    this->send_kills();

    simgrid::s4u::this_actor::exit();
}


/* Sends the global model to every start_nodes */
void SimpleAggregator::send_global_model()
{
    auto nm = this->get_network_manager();
    Packet *p;

    auto args = new std::unordered_map<std::string, std::string>();
    args->insert({ "number_local_epochs", std::to_string(this->number_local_epochs) });

    // Sadly we cannot use smart pointers in mailbox->put because it takes a void* as parameter...
    p = new Packet(Packet::Operation::SEND_GLOBAL_MODEL, this->get_network_manager()->get_my_node_name(), "BROADCAST", args);

    uint16_t nb_sent = nm->broadcast(p, Filters::trainers);

    this->number_client_training = nb_sent;
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
        p = nm->get();

        // Note that here we don't check that the local models come from different trainers
        if (p->op == Packet::Operation::SEND_LOCAL_MODEL)
        {
            number_local_models += 1;
        }
    }
    
    XBT_INFO("Retrieved %lu local models out of %lu", number_local_models, this->number_client_training);

    return number_local_models;
}

void SimpleAggregator::send_kills()
{
    auto nm = this->get_network_manager();
    Packet *p = new Packet(Packet::Operation::KILL_TRAINER, this->get_network_manager()->get_my_node_name(), "BROADCAST");

    nm->broadcast(p, Filters::trainers);
}
