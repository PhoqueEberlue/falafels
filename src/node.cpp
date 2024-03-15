#include <cstdlib>
#include <memory>
#include <simgrid/forward.h>
#include <simgrid/s4u/Actor.hpp>
#include <string>
#include <xbt/log.h>

#include "./node.hpp"
#include "constants.hpp"
#include "protocol.hpp"

XBT_LOG_NEW_DEFAULT_CATEGORY(s4u_node, "Messages specific for this example");

Aggregator::Aggregator(std::string host_name, std::vector<std::string> start_nodes)
{
    this->mailbox = simgrid::s4u::Mailbox::by_name(host_name);
    this->start_nodes = start_nodes;
    this->host_name = host_name;
}

Trainer::Trainer(std::string host_name, std::vector<std::string> start_nodes)
{
    this->mailbox = simgrid::s4u::Mailbox::by_name(host_name);
    this->start_nodes = start_nodes;
    this->host_name = host_name;
}

Proxy::Proxy(std::string host_name, std::vector<std::string> start_nodes)
{
    this->mailbox = simgrid::s4u::Mailbox::by_name(host_name);
    this->start_nodes = start_nodes;
    this->host_name = host_name;
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
    for (std::string node_name: this->start_nodes)
    {   
        simgrid::s4u::Mailbox *mailbox = simgrid::s4u::Mailbox::by_name(node_name);

        // Sadly we cannot use smart pointers in mailbox->put because it takes a void* as parameter...
        packet *p = new packet {.op=packet::operation::SEND_GLOBAL_MODEL, .src=this->host_name};

        XBT_INFO("Sending '%s' to %s", operation_to_str(p->op), node_name.c_str());

        mailbox->put(p, constants::MODEL_SIZE_BYTES);
    }
}

void Aggregator::wait_local_models()
{
    for (auto node_name: this->start_nodes)
    {
        packet *p = this->mailbox->get<packet>();
        XBT_INFO("Received message: %s", operation_to_str(p->op));

        if (p->op == packet::operation::SEND_LOCAL_MODEL)
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
    for (auto node_name: this->start_nodes)
    {
        simgrid::s4u::Mailbox *mailbox = simgrid::s4u::Mailbox::by_name(node_name);

        // auto p = std::make_unique<packet>(packet::operation::KILL_TRAINER, this->host_name);
        packet *p = new packet {.op=packet::operation::KILL_TRAINER, .src=this->host_name};

        XBT_INFO("Sending '%s' to %s", operation_to_str(p->op), node_name.c_str());

        mailbox->put(p, sizeof(*p));
    }
}

void Trainer::run()
{
    packet *p = this->mailbox->get<packet>();

    while (p->op != packet::operation::KILL_TRAINER)
    {
        XBT_INFO("Received message: %s", operation_to_str(p->op));

        if (p->op == packet::operation::SEND_GLOBAL_MODEL)
        {
            double flops = constants::trainer::LOCAL_EPOCH_FLOPS;
            XBT_INFO("Starting local training with flops value: %f", flops);

            simgrid::s4u::this_actor::execute(flops);

            // Retrieving src's mailbox
            auto source_mailbox = simgrid::s4u::Mailbox::by_name(p->src);

            // auto res_p = std::make_unique<packet>(packet::operation::SEND_LOCAL_MODEL, this->host_name);
            packet *res_p = new packet {.op=packet::operation::SEND_LOCAL_MODEL, .src=this->host_name};

            XBT_INFO("Sending local model to: %s", p->src.c_str());
            source_mailbox->put(res_p, constants::MODEL_SIZE_BYTES);
        }

        // Delete current packet and wait for another one 
        delete p;
        p = this->mailbox->get<packet>();
    }

    // Delete kill packte 
    delete p;

    simgrid::s4u::this_actor::exit();
}

void Proxy::run() {}
