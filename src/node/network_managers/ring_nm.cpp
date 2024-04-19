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

RingNetworkManager::RingNetworkManager(NodeInfo *node_info)
{
    this->my_node_info = node_info;

    // Initializing our mailbox
    this->mailbox = simgrid::s4u::Mailbox::by_name(node_info->name);

    auto tmp = new simgrid::s4u::ActivitySet();
    this->pending_comms = simgrid::s4u::ActivitySetPtr(tmp);
    this->received_packets = std::vector<packet_id>();
}

RingNetworkManager::~RingNetworkManager()
{
    delete this->left_node;
    delete this->right_node;
    delete this->my_node_info;
    delete this->bootstrap_nodes;
    delete this->my_node_info;
};

void RingNetworkManager::handle_registration_requests()
{
//     xbt_assert(this->my_node_info->role == NodeRole::Aggregator);
// 
//     std::unique_ptr<Packet> p;
// 
//     while (true) 
//     {
//         try 
//         {
//             p = this->get(Constants::REGISTRATION_TIMEOUT);
//         }
//         catch (simgrid::TimeoutException)
//         {
//             break;
//         }
// 
//         if (p->op == Packet::Operation::REGISTRATION_REQUEST)
//         {
//             xbt_assert(p->node_info != nullptr, "field node_info should be valid when Packet are with REGISTRATION_REQUEST operation");
// 
//             this->connected_nodes.push_back(p->node_info);
// 
//             XBT_INFO("Added %s as a connected node", p->node_info->name.c_str());
// 
//             Packet *res_p = new Packet(Packet::Operation::REGISTRATION_CONFIRMATION, this->get_my_node_name(), p->final_dst);
//             this->send(res_p, p->src);
//         }
//     }
//     this->number_connected_nodes = this->connected_nodes->size();
}

void RingNetworkManager::send_registration_request()
{
    xbt_assert(this->my_node_info->role == NodeRole::Trainer);

    // Take the first bootstrap node, in a centralized topology we should have only one anyways.
    auto bootstrap_node = this->bootstrap_nodes->at(0);

    Packet *p = new Packet(
        this->get_my_node_name(), bootstrap_node->name, 
        Packet::Operation::REGISTRATION_REQUEST,
        new Packet::Data { .node_to_register = *this->my_node_info }
    );

    this->send(p, bootstrap_node->name);

    std::unique_ptr<Packet> response;

    while (true) 
    {
        response = this->get();

        if (response->op == Packet::Operation::REGISTRATION_CONFIRMATION)
        {
            XBT_INFO("Succesfully registered");
            break;
        }
    }
}

uint16_t RingNetworkManager::broadcast(Packet *packet, FilterNode filter) 
{
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
    // packet->decr_ref_count();

    return res;
}

uint16_t RingNetworkManager::broadcast(Packet *packet, FilterNode filter, uint64_t timeout)
{
    // TODO
}

std::unique_ptr<Packet> RingNetworkManager::get()
{
//     // How can I handle that correctly
//     auto completed_one = this->pending_comms->test_any();
// 
//     if (completed_one != nullptr)
//     {
// 
//     }
// 
//     std::unique_ptr<Packet> p;
//     bool cond = true;
// 
//     while (cond)
//     {
//         p = this->mailbox->get_unique<Packet>();
//         XBT_INFO("%s <--%s--- %s", this->get_my_node_name().c_str(), p->op_string.c_str(), p->src.c_str()); 
// 
//         auto v = this->received_packets;
// 
//         // Don't redirect packets as an aggregator
//         if (this->my_node_info->role != NodeRole::Aggregator)
//         {
//             // if the packet was allready received 
//             if(std::find(v.begin(), v.end(), p->id) != v.end()) 
//             {
//                 XBT_INFO("discarding packet %lu", p->id);
//                 // ignore
//             } 
//             else 
//             {
//                 // if the packet has broadcast enabled or if dst is not for the current node
//                 if (p->final_dst == "BROADCAST" || p->final_dst != this->get_my_node_name())
//                 {
//                     // TODO: redirect 
//                     // this->redirect(p);
//                 }
// 
//                 // Add to received packets
//                 this->received_packets.push_back(p->id);
//                 cond = false;
//             }
//         }
//         else 
//         {
//             cond = false;
//         }
//     }
// 
//     return p;
}

void RingNetworkManager::redirect(Packet *p)
{
//    // Clone the packet because we need to modify the source
//    auto new_p = p->clone();
//    // Set the new source to the current node name
//    new_p->src = this->get_my_node_name();
//
//    node_name dst;
//    // If the packet comes from our left
//    if (p->src == this->left_node->name) 
//    {
//        // Send to the right
//        dst = this->right_node->name;
//    }
//    else 
//    {
//        // Send to the left 
//        dst = this->left_node->name;
//    }
//    
//    auto comm = this->send_async(new_p, dst);
//    this->pending_comms->push(comm);
}

void RingNetworkManager::wait_last_comms() 
{
    this->pending_comms->wait_all();
}

std::unique_ptr<Packet> RingNetworkManager::get(double timeout)
{
//     auto p = this->mailbox->get_unique<Packet>(timeout);
//     XBT_INFO("%s <--%s--- %s", this->get_my_node_name().c_str(), p->op_string.c_str(), p->src.c_str());
//     return p;
}
