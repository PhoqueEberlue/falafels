#include <simgrid/s4u/Engine.hpp>
#include <simgrid/s4u/Mailbox.hpp>
#include <unordered_map>
#include <vector>
#include <xbt/log.h>

#include "asynchronous_aggregator.hpp"
#include "../../../protocol.hpp"
#include "../../../utils/utils.hpp"

XBT_LOG_NEW_DEFAULT_CATEGORY(s4u_asynchronous_aggregator, "Messages specific for this example");

static bool trainer_filter(NodeInfo *node_info)
{
    return node_info->role == NodeRole::Trainer;
}

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
    // At start we suppose that every trainer nodes are available
    available_trainers = this->get_network_manager()->get_node_names_filter(trainer_filter);
    total_number_clients = available_trainers.size();

    while (simgrid::s4u::Engine::get_instance()->get_clock() < 2)
    {
        this->send_global_model_to_available_trainers();
        uint64_t number_local_models = this->wait_local_models();
        this->aggregate(number_local_models);
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
        auto args = new std::unordered_map<std::string, std::string>();
        args->insert({ "number_local_epochs", std::to_string(this->number_local_epochs) });
        p = new Packet { .op=Packet::Operation::SEND_GLOBAL_MODEL, .src=nm->get_my_node_name(), .args=args };

        XBT_INFO("%s ---%s--> %s", p->src.c_str(), operation_to_str(p->op), node_name.c_str());

        nm->put(p, node_name);
    }

    // The trainers are not available anymore, clearing them
    this->available_trainers.clear();
}

uint64_t AsynchronousAggregator::wait_local_models()
{
    uint64_t number_local_models = 0;
    auto nm = this->get_network_manager();
    Packet *p;

    while (true)
    {
        // When proportion threshold is set to 0, wait for at least one model
        if (this->proportion_threshold == 0.0)
        {
            if (number_local_models == 1)
                break;

        } 
        // Else wait for the proportion threshold to be met
        else 
        {
            if (number_local_models >= this->total_number_clients * this->proportion_threshold)
                break;
        }

        p = nm->get();
        XBT_INFO("%s <--%s--- %s", nm->get_my_node_name().c_str(), operation_to_str(p->op), p->src.c_str());

        // Note that here we don't check that the local models come from different trainers
        if (p->op == Packet::Operation::SEND_LOCAL_MODEL)
        {
            // Add the sender to the available trainers list
            this->available_trainers.push_back(p->src);
            number_local_models += 1;
        }
        
        delete p;
    }
    
    XBT_INFO("Enough local models: %lu out of %i with proportion threshold of %f", number_local_models, this->total_number_clients, this->proportion_threshold);

    return number_local_models;
}

void AsynchronousAggregator::send_kills()
{
    auto nm = this->get_network_manager();
    Packet *p;

    for (auto node_name: nm->get_node_names_filter(trainer_filter))
    {
        p = new Packet { .op=Packet::Operation::KILL_TRAINER, .src=nm->get_my_node_name() };

        XBT_INFO("%s ---%s--> %s", p->src.c_str(), operation_to_str(p->op), node_name.c_str());

        nm->put(p, node_name);
    }
}
