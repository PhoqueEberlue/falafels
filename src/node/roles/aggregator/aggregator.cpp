#include <simgrid/engine.h>
#include <simgrid/forward.h>
#include <simgrid/s4u/Actor.hpp>
#include <simgrid/s4u/Engine.hpp>
#include <xbt/log.h>

#include "aggregator.hpp"
#include "../../../protocol.hpp"
#include "../../../constants.hpp"

XBT_LOG_NEW_DEFAULT_CATEGORY(s4u_aggregator, "Messages specific for this example");

Aggregator::Aggregator() {}

bool trainer_filter(NodeInfo *node_info) {
    return node_info->role == NodeRole::Trainer;
}

void Aggregator::run()
{
    while (simgrid::s4u::Engine::get_instance()->get_clock() < 2)
    {
        this->send_global_model();
        this->wait_local_models();
    } 

    this->send_kills();

    simgrid::s4u::this_actor::exit();
}

/* Sends the global model to every start_nodes */
void Aggregator::send_global_model()
{
    for (std::string node_name: this->get_network_manager()->get_node_names_filter(trainer_filter))
    {   
        simgrid::s4u::Mailbox *mailbox = simgrid::s4u::Mailbox::by_name(node_name);

        // Sadly we cannot use smart pointers in mailbox->put because it takes a void* as parameter...
        Packet *p = new Packet { .op=Packet::Operation::SEND_GLOBAL_MODEL, .src=this->get_network_manager()->get_my_host_name() };

        XBT_INFO("%s -> %s", operation_to_str(p->op), node_name.c_str());

        mailbox->put(p, constants::MODEL_SIZE_BYTES);
    }
}

void Aggregator::wait_local_models()
{
    auto nm = this->get_network_manager();
    for (auto node_name: nm->get_node_names_filter(trainer_filter))
    {
        Packet *p = nm->get();
        XBT_INFO("%s <- %s", nm->get_my_host_name().c_str(), operation_to_str(p->op));

        if (p->op == Packet::Operation::SEND_LOCAL_MODEL)
        {
            delete p;
            double flops = constants::aggregator::AGGREGATION_FLOPS;
            XBT_INFO("Starting aggregation with flops value: %f", flops);
            simgrid::s4u::this_actor::execute(flops);
        }

    }
}

void Aggregator::send_kills()
{
    for (auto node_name: this->get_network_manager()->get_node_names_filter(trainer_filter))
    {
        simgrid::s4u::Mailbox *mailbox = simgrid::s4u::Mailbox::by_name(node_name);

        Packet *p = new Packet { .op=Packet::Operation::KILL_TRAINER, .src=this->get_network_manager()->get_my_host_name() };

        XBT_INFO("%s -> %s", operation_to_str(p->op), node_name.c_str());

        mailbox->put(p, sizeof(*p));
    }
}
