#include <cstdint>
#include <simgrid/Exception.hpp>
#include <simgrid/forward.h>
#include <vector>
#include <xbt/asserts.h>
#include <xbt/ex.h>
#include <xbt/log.h>
#include "ring_nm.hpp"

XBT_LOG_NEW_DEFAULT_CATEGORY(s4u_ring_nm, "Messages specific for this example");

RingNetworkManager::RingNetworkManager(node_name name, NodeRole role)
{
    this->my_node_name = name;
    this->my_node_role = role;

    // Initializing our mailbox
    this->mailbox = simgrid::s4u::Mailbox::by_name(name);

    auto tmp = new simgrid::s4u::ActivitySet();
    this->pending_comms = simgrid::s4u::ActivitySetPtr(tmp);
    this->received_packets = std::vector<packet_id>();
}

RingNetworkManager::~RingNetworkManager() {};

void RingNetworkManager::set_bootstrap_nodes(std::vector<NodeInfo*> *nodes)
{
    xbt_assert(nodes->size() == 2, "The RingNetworkManager must have at least 2 bootstrap nodes");
    this->left_node = nodes->at(0);
    this->right_node = nodes->at(1);
}

uint16_t RingNetworkManager::broadcast(Packet *packet, FilterNode filter) {
    uint16_t res = 0;

    // Send to left node
    if (filter(this->left_node))
    {
        this->send(packet, this->left_node->name);
        res++;
    }

    // Send to right node
    if (filter(this->right_node))
    {
        this->send(packet, this->right_node->name);
        res++;
    }

    // Delete our own reference of the packet
    packet->decr_ref_count();

    return res;
}

uint16_t RingNetworkManager::broadcast(Packet *packet, FilterNode filter, uint64_t timeout)
{
    // TODO
}

Packet *RingNetworkManager::get()
{
    // How can I handle that correctly
    auto completed_one = this->pending_comms->test_any();

    if (completed_one != nullptr)
    {

    }

    Packet *p;
    bool cond = true;

    while (cond)
    {
        p = this->mailbox->get<Packet>();
        XBT_INFO("%s <--%s--- %s", this->my_node_name.c_str(), p->op_string.c_str(), p->src.c_str()); 

        auto v = this->received_packets;

        // Don't redirect packets as an aggregator
        if (this->my_node_role != NodeRole::Aggregator)
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
                if (p->final_dst == "BROADCAST" || p->final_dst != this->my_node_name)
                {
                    // redirect 
                    this->redirect(p);
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

void RingNetworkManager::redirect(Packet *p)
{
    // Clone the packet because we need to modify the source
    auto new_p = p->clone();
    // Set the new source to the current node name
    new_p->src = this->my_node_name;

    node_name dst;
    // If the packet comes from our left
    if (p->src == this->left_node->name) 
    {
        // Send to the right
        dst = this->right_node->name;
    }
    else 
    {
        // Send to the left 
        dst = this->left_node->name;
    }
    
    auto comm = this->send_async(new_p, dst);
    this->pending_comms->push(comm);
}

void RingNetworkManager::wait_last_comms() 
{
    this->pending_comms->wait_all();
}
