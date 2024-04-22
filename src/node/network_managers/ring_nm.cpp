#include <cstdint>
#include <memory>
#include <simgrid/Exception.hpp>
#include <simgrid/forward.h>
#include <vector>
#include <xbt/asserts.h>
#include <xbt/ex.h>
#include <xbt/log.h>
#include "ring_nm.hpp"
#include <simgrid/s4u/Engine.hpp>

XBT_LOG_NEW_DEFAULT_CATEGORY(s4u_ring_nm, "Messages specific for this example");

RingNetworkManager::RingNetworkManager(NodeInfo node_info)
{
    this->my_node_info = node_info;

    // Initializing our mailbox
    this->mailbox = simgrid::s4u::Mailbox::by_name(node_info.name);

    auto tmp = new simgrid::s4u::ActivitySet();
    this->pending_comms = simgrid::s4u::ActivitySetPtr(tmp);
    this->received_packets = std::vector<packet_id>();
}

RingNetworkManager::~RingNetworkManager()
{
    delete this->bootstrap_nodes;
};

uint16_t RingNetworkManager::handle_registration_requests()
{
    xbt_assert(this->my_node_info.role == NodeRole::Aggregator);
 
    auto node_list_tmp = std::vector<NodeInfo>();
    
    std::unique_ptr<Packet> p;
 
    // ---------- Wait for registrations ----------
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
            node_list_tmp.push_back(*p->data->node_to_register); 
        }
    }

    NodeInfo left_n;
    NodeInfo right_n;

    // ---------- Create connections and send them ----------
    for (int i = 0; i < node_list_tmp.size(); i++)
    {
        // Handle edges -> link to the current node to close the ring
        if (i == 0)
        {
            left_n = this->my_node_info;
            right_n = node_list_tmp.at(i + 1);
            this->right_node = node_list_tmp.at(i);
        }
        else if (i == node_list_tmp.size() - 1) 
        {
            left_n = node_list_tmp.at(i - 1);
            right_n = this->my_node_info;
            this->left_node = node_list_tmp.at(i);
        }
        // Normal case we take i-1 and i+1 as neigbours
        else 
        {
            left_n = node_list_tmp.at(i - 1);
            right_n = node_list_tmp.at(i + 1);
        }

        auto neigbours = new std::vector<NodeInfo>();
        neigbours->push_back(left_n);
        neigbours->push_back(right_n);

        Packet *res_p = new Packet(
            this->get_my_node_name(), 
            node_list_tmp.at(i).name, // Sent the packt to the current node
            Packet::Operation::REGISTRATION_CONFIRMATION, 
            new Packet::Data { .node_list = neigbours }
        );

        this->send(res_p, node_list_tmp.at(i).name);
    }

    XBT_INFO("Aggregator left node: %s", this->left_node.name.c_str());
    XBT_INFO("Aggregator right node: %s", this->right_node.name.c_str());

    // Return the number of nodes that have been registered
    return node_list_tmp.size();
}

void RingNetworkManager::send_registration_request()
{
    xbt_assert(this->my_node_info.role == NodeRole::Trainer);

    // Take the first bootstrap node. TODO: handle multiple bootstrap nodes??
    auto bootstrap_node = this->bootstrap_nodes->at(0);

    Packet *p = new Packet(
        this->get_my_node_name(), bootstrap_node->name, 
        Packet::Operation::REGISTRATION_REQUEST,
        new Packet::Data { .node_to_register = &this->my_node_info }
    );

    // Send the request
    this->send(p, bootstrap_node->name);

    std::unique_ptr<Packet> response;

    // Wait for confirmation 
    while (true) 
    {
        response = this->get();

        if (response->op == Packet::Operation::REGISTRATION_CONFIRMATION)
        {
            XBT_INFO("Succesfully registered");
            this->left_node = response->data->node_list->at(0);
            this->right_node = response->data->node_list->at(1);
            XBT_INFO("Adding %s as left node", this->left_node.name.c_str());
            XBT_INFO("Adding %s as right node", this->right_node.name.c_str());
            break;
        }
    }
}

uint16_t RingNetworkManager::broadcast(Packet *packet, FilterNode filter) 
{
    uint16_t res = 0;

    // Send to left node
    if (filter(&this->left_node))
    {
        this->send(packet->clone(), this->left_node.name);
        res++;
    }

    // Send to right node
    if (filter(&this->right_node))
    {
        this->send(packet->clone(), this->right_node.name);
        res++;
    }

    // Delete our version of the packet
    delete packet;

    return res;
}

uint16_t RingNetworkManager::broadcast(Packet *packet, FilterNode filter, uint64_t timeout)
{
    // TODO
}

std::unique_ptr<Packet> RingNetworkManager::get()
{
    // How can I handle that correctly
    auto completed_one = this->pending_comms->test_any();
 
    if (completed_one != nullptr)
    {
 
    }
 
    std::unique_ptr<Packet> p;
    bool cond = true;
 
    while (cond)
    {
        p = this->mailbox->get_unique<Packet>();
        XBT_INFO("%s <--%s--- %s", this->get_my_node_name().c_str(), p->op_string.c_str(), p->src.c_str()); 
 
        auto v = this->received_packets;
 
        // Don't redirect packets as an aggregator
        if (this->my_node_info.role != NodeRole::Aggregator)
        {
            // if the packet was allready received 
            if(std::find(v.begin(), v.end(), p->id) != v.end()) 
            {
                XBT_INFO("discarding packet %lu", p->id);
                // ignore
            } 
            else 
            {
                // if the packet has broadcast enabled or if dst is not for the current node
                if (p->final_dst == "BROADCAST" || p->final_dst != this->get_my_node_name())
                {
                    this->redirect(p);
                }

                
                if (p->op == Packet::Operation::KILL_TRAINER)
                {
                    try
                    {
                        // Wait finish pending comms before exiting with timeout in case of double send
                        this->pending_comms->wait_all_for(2);
                    } 
                    catch (simgrid::TimeoutException) {
                        XBT_INFO("Timeout");
                    }
                }
 
                // Add to received packets
                this->received_packets.push_back(p->id);
                cond = false;
            }
        }
        else 
        {
            cond = false;
        }
    }
 
    return p;
}

void RingNetworkManager::redirect(std::unique_ptr<Packet> &p)
{
    // Clone the packet because we need to modify the source
    auto new_p = p->clone();
    // Set the new source to the current node name
    new_p->src = this->get_my_node_name();

    node_name dst;
    // If the original packet came from our left
    if (p->src == this->left_node.name) 
    {
        // Send to the right
        dst = this->right_node.name;
    }
    else 
    {
        // Send to the left 
        dst = this->left_node.name;
    }
    
    // Note that the final dest stays the same.
    auto comm = this->send_async(new_p, dst);
    this->pending_comms->push(comm);
}

// void RingNetworkManager::wait_last_comms() 
// {
//     this->pending_comms->wait_all();
// }

std::unique_ptr<Packet> RingNetworkManager::get(double timeout)
{
     auto p = this->mailbox->get_unique<Packet>(timeout);
     XBT_INFO("%s <--%s--- %s", this->get_my_node_name().c_str(), p->op_string.c_str(), p->src.c_str());
     return p;
}
