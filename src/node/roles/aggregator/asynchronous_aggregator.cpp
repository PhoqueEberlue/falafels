#include <simgrid/s4u/Engine.hpp>
#include <vector>
#include <xbt/log.h>

#include "asynchronous_aggregator.hpp"
#include "../../../protocol.hpp"
#include "../../../constants.hpp"

XBT_LOG_NEW_DEFAULT_CATEGORY(s4u_asynchronous_aggregator, "Messages specific for this example");

static bool trainer_filter(NodeInfo *node_info)
{
    return node_info->role == NodeRole::Trainer;
}

AsynchronousAggregator::AsynchronousAggregator()
{
}

void AsynchronousAggregator::run()
{
    // At start we suppose that every trainer nodes are available
    available_trainers = this->get_network_manager()->get_node_names_filter(trainer_filter);
    total_number_clients = available_trainers.size();

    while (simgrid::s4u::Engine::get_instance()->get_clock() < 2)
    {
        this->send_global_model_to_available_trainers();
        this->wait_local_models();
        this->aggregate();
    } 

    this->send_kills();

    simgrid::s4u::this_actor::exit();
}

/* Sends the global model to every start_nodes */
void AsynchronousAggregator::send_global_model_to_available_trainers()
{
    auto nm = this->get_network_manager();
    Packet *p;

    for (std::string node_name: this->available_trainers)
    {   
        p = new Packet { .op=Packet::Operation::SEND_GLOBAL_MODEL, .src=nm->get_my_node_name() };

        XBT_INFO("%s -> %s", operation_to_str(p->op), node_name.c_str());

        nm->put(p, node_name, constants::MODEL_SIZE_BYTES);
    }

    // The trainers are not available anymore, clearing them
    this->available_trainers.clear();
}

void AsynchronousAggregator::wait_local_models()
{
    uint64_t number_local_models = 0;
    auto nm = this->get_network_manager();
    Packet *p;

    // While we don't have enough local models
    while (number_local_models < this->total_number_clients * this->proportion_threshold)
    {
        p = nm->get();
        XBT_INFO("%s <- %s", nm->get_my_node_name().c_str(), operation_to_str(p->op));

        // Note that here we don't check that the local models come from different trainers
        if (p->op == Packet::Operation::SEND_LOCAL_MODEL)
        {
            // Add the sender to the available trainers list
            this->available_trainers.push_back(p->src);
            number_local_models += 1;
        }
    }
    
    XBT_INFO("Enough local models: %lu out of %i with proportion threshold of %f", number_local_models, this->total_number_clients, this->proportion_threshold);
}

void AsynchronousAggregator::send_kills()
{
    auto nm = this->get_network_manager();
    Packet *p;

    for (auto node_name: nm->get_node_names_filter(trainer_filter))
    {
        p = new Packet { .op=Packet::Operation::KILL_TRAINER, .src=nm->get_my_node_name() };

        XBT_INFO("%s -> %s", operation_to_str(p->op), node_name.c_str());

        nm->put(p, node_name, sizeof(*p));
    }
}
