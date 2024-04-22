#include "star_nm.hpp"
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iterator>
#include <memory>
#include <ostream>
#include <simgrid/Exception.hpp>
#include <simgrid/s4u/Engine.hpp>
#include <simgrid/s4u/Mailbox.hpp>
#include <vector>
#include <xbt/asserts.h>
#include <xbt/log.h>

XBT_LOG_NEW_DEFAULT_CATEGORY(s4u_star_nm, "Messages specific for this example");

StarNetworkManager::StarNetworkManager(NodeInfo node_info)
{
    this->my_node_info = node_info;
    // Initializing our mailbox
    this->mailbox = simgrid::s4u::Mailbox::by_name(this->my_node_info.name);
    this->connected_nodes = new std::vector<NodeInfo>();
}

StarNetworkManager::~StarNetworkManager() 
{
    delete this->connected_nodes;
    delete this->bootstrap_nodes;
};

uint16_t StarNetworkManager::handle_registration_requests()
{
    xbt_assert(this->my_node_info.role == NodeRole::Aggregator);

    uint16_t number_registration = 0;
    std::unique_ptr<Packet> p;

    while (true) 
    {
        try 
        {
            p = this->get(Constants::REGISTRATION_TIMEOUT);
        }
        catch (simgrid::TimeoutException)
        {
            break;
        }

        if (p->op == Packet::Operation::REGISTRATION_REQUEST)
        {
            this->connected_nodes->push_back(*p->data->node_to_register);

            XBT_INFO("Added %s as a connected node", p->data->node_to_register->name.c_str());

            auto node_list = new std::vector<NodeInfo>();
            node_list->push_back(this->my_node_info);

            Packet *res_p = new Packet(
                this->get_my_node_name(), p->final_dst,
                Packet::Operation::REGISTRATION_CONFIRMATION,
                new Packet::Data { .node_list=node_list }
            );

            this->send(res_p, p->src);

            number_registration += 1;
        }
    }

    return number_registration;
}

void StarNetworkManager::send_registration_request()
{
    xbt_assert(this->my_node_info.role == NodeRole::Trainer);

    // Take the first bootstrap node, in a centralized topology we should have only one anyways.
    auto bootstrap_node = this->bootstrap_nodes->at(0);

    Packet *p = new Packet(
        this->get_my_node_name(), bootstrap_node->name,
        Packet::Operation::REGISTRATION_REQUEST,
        new Packet::Data { .node_to_register = &this->my_node_info } 
    );

    this->send(p, bootstrap_node->name);

    std::unique_ptr<Packet> response;

    while (true) 
    {
        response = this->get();

        if (response->op == Packet::Operation::REGISTRATION_CONFIRMATION)
        {
            for (auto node: *response->data->node_list)
            {
                this->connected_nodes->push_back(node);
                XBT_INFO("Add connection to %s", node.name.c_str());
            }

            break;
        }
    }
}

uint16_t StarNetworkManager::broadcast(Packet *packet, FilterNode filter)
{
    Packet *p_clone;

    for(auto node_info : *this->connected_nodes)
    {
        if (filter(&node_info))
        {
            p_clone = packet->clone();
            this->send(p_clone, node_info.name);
        }
    }

    // Delete our own packet
    delete packet;

    return this->connected_nodes->size();
}

uint16_t StarNetworkManager::broadcast(Packet *packet, FilterNode filter, uint64_t timeout)
{
    uint16_t nb_sent = 0;
    Packet *p_clone;

    for(auto node_info : *this->connected_nodes)
    {
        if (filter(&node_info))
        {
            p_clone = packet->clone();
            if (send(p_clone, node_info.name, timeout))
                nb_sent++;
        }
    }

    // Delete our own packet
    delete packet;

    return nb_sent;
}

std::unique_ptr<Packet> StarNetworkManager::get()
{
    auto p = this->mailbox->get_unique<Packet>();
    XBT_INFO("%s <--%s--- %s", this->get_my_node_name().c_str(), p->op_string.c_str(), p->original_src.c_str());
    return p;
}

std::unique_ptr<Packet> StarNetworkManager::get(double timeout)
{
    auto p = this->mailbox->get_unique<Packet>(timeout);
    XBT_INFO("%s <--%s--- %s", this->get_my_node_name().c_str(), p->op_string.c_str(), p->original_src.c_str());
    return p;
}
