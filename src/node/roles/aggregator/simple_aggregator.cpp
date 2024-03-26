#include <simgrid/s4u/Engine.hpp>
#include <xbt/log.h>

#include "simple_aggregator.hpp"
#include "../../../protocol.hpp"
#include "../../../constants.hpp"

XBT_LOG_NEW_DEFAULT_CATEGORY(s4u_simple_aggregator, "Messages specific for this example");

SimpleAggregator::SimpleAggregator()
{
}

static bool trainer_filter(NodeInfo *node_info)
{
    return node_info->role == NodeRole::Trainer;
}

void SimpleAggregator::run()
{
    while (simgrid::s4u::Engine::get_instance()->get_clock() < 2)
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
    this->number_client_training = 0;
    auto nm = this->get_network_manager();
    Packet *p;

    for (std::string node_name: nm->get_node_names_filter(trainer_filter))
    {   
        auto args = new std::unordered_map<std::string, std::string>();
        args->insert({ "number_local_epochs", std::to_string(this->number_local_epochs) });

        // Sadly we cannot use smart pointers in mailbox->put because it takes a void* as parameter...
        p = new Packet { .op=Packet::Operation::SEND_GLOBAL_MODEL, .src=nm->get_my_node_name(), .args=args };

        XBT_INFO("%s -> %s", operation_to_str(p->op), node_name.c_str());

        nm->put(p, node_name, constants::MODEL_SIZE_BYTES);

        this->number_client_training += 1;
    }
}

uint64_t SimpleAggregator::wait_local_models()
{
    uint64_t number_local_models = 0;
    double flops                 = constants::aggregator::AGGREGATION_FLOPS;
    auto nm                      = this->get_network_manager();
    Packet *p;

    // While we don't have enough local models
    while (number_local_models < this->number_client_training)
    {
        p = nm->get();
        XBT_INFO("%s <- %s", nm->get_my_node_name().c_str(), operation_to_str(p->op));

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
    Packet *p;

    for (auto node_name: nm->get_node_names_filter(trainer_filter))
    {
        p = new Packet { .op=Packet::Operation::KILL_TRAINER, .src=nm->get_my_node_name() };

        XBT_INFO("%s -> %s", operation_to_str(p->op), node_name.c_str());

        nm->put(p, node_name, sizeof(*p));
    }
}
