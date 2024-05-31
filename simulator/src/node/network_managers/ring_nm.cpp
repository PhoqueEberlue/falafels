#include <cstdint>
#include <format>
#include <memory>
#include <simgrid/Exception.hpp>
#include <simgrid/forward.h>
#include <vector>
#include <xbt/asserts.h>
#include <xbt/ex.h>
#include <xbt/log.h>
#include <simgrid/s4u/Engine.hpp>

#include "ring_nm.hpp"
#include "../../dot.hpp"
#include "nm.hpp"

using namespace std;

XBT_LOG_NEW_DEFAULT_CATEGORY(s4u_ring_nm, "Messages specific for this example");

RingNetworkManager::RingNetworkManager(NodeInfo node_info) : NetworkManager(node_info)
{ 
    this->received_packet_ids = new vector<packet_id>();
}

RingNetworkManager::~RingNetworkManager()
{
    delete this->received_packet_ids;
};

void RingNetworkManager::handle_registration_requests()
{
    xbt_assert(this->my_node_info.role == NodeRole::Aggregator);
 
    DOTGenerator::get_instance().add_to_cluster(
        std::format("cluster-{}", this->my_node_info.name),
        std::format("{} [label=\"{}\", color=green]", this->my_node_info.name, this->my_node_info.name)
    );

    NodeInfo left_n;
    NodeInfo right_n;

    // ---------- Creating connections ----------
    for (int i = 0; i < this->registration_requests->size(); i++)
    {
        // Handle edges -> link to the current node to close the ring
        if (i == 0)
        {
            left_n = this->my_node_info;
            right_n = this->registration_requests->at(i + 1).node_to_register;
            this->right_node = this->registration_requests->at(i).node_to_register;
        }
        else if (i == this->registration_requests->size() - 1) 
        {
            left_n = this->registration_requests->at(i - 1).node_to_register;
            right_n = this->my_node_info;
            this->left_node = this->registration_requests->at(i).node_to_register;
        }
        // Normal case we take i-1 and i+1 as neigbours
        else 
        {
            left_n = this->registration_requests->at(i - 1).node_to_register;
            right_n = this->registration_requests->at(i + 1).node_to_register;
        } 

        auto neigbours = vector<NodeInfo>();
        neigbours.push_back(left_n);
        neigbours.push_back(right_n);

        auto res_p = make_shared<Packet>(Packet(
            this->registration_requests->at(i).node_to_register.name, // Sent the packet to the current node
            this->registration_requests->at(i).node_to_register.name, // Sent the packet to the current node
            Packet::RegistrationConfirmation(
                make_shared<vector<NodeInfo>>(neigbours)
            )
        ));

        // Sending the packet
        this->send_async(res_p);
    } 

    DOTGenerator::get_instance().add_to_cluster(
        std::format("cluster-{}", this->my_node_info.name),
        std::format("{} -> {} [color=green]", this->my_node_info.name, this->left_node.name)
    );

    DOTGenerator::get_instance().add_to_cluster(
        std::format("cluster-{}", this->my_node_info.name),
        std::format("{} -> {} [color=green]", this->my_node_info.name, this->right_node.name)
    );

    this->mp->put_nm_event(
        Mediator::ClusterConnected { .number_client_connected=(uint16_t)this->registration_requests->size() }
    );
}

void RingNetworkManager::send_registration_request()
{
    xbt_assert(this->my_node_info.role == NodeRole::Trainer);

    // Take the first bootstrap node. TODO: handle multiple bootstrap nodes??
    auto bootstrap_node = this->bootstrap_nodes->at(0);

    auto p = make_shared<Packet>(Packet(
        bootstrap_node.name, bootstrap_node.name, 
        Packet::RegistrationRequest(
            this->my_node_info
        )
    ));

    // Send the request
    this->send_async(p);
}

void RingNetworkManager::handle_registration_confirmation(const Packet::RegistrationConfirmation &confirmation)
{
    XBT_INFO("Succesfully registered");

    auto bootstrap_node = this->bootstrap_nodes->at(0);

    this->left_node = confirmation.node_list->at(0);
    this->right_node = confirmation.node_list->at(1);

    DOTGenerator::get_instance().add_to_cluster(
        std::format("cluster-{}", bootstrap_node.name),
        std::format("{} [label=\"{}\", color=yellow]", this->my_node_info.name, this->my_node_info.name)
    );

    DOTGenerator::get_instance().add_to_cluster(
        std::format("cluster-{}", bootstrap_node.name),
        std::format("{} -> {} [color=green]", this->my_node_info.name, this->left_node.name)
    );

    DOTGenerator::get_instance().add_to_cluster(
        std::format("cluster-{}", bootstrap_node.name),
        std::format("{} -> {} [color=green]", this->my_node_info.name, this->right_node.name)
    );

    this->mp->put_nm_event(Mediator::NodeConnected {});
}

void RingNetworkManager::route_packet(std::unique_ptr<Packet> packet)
{
    // Check that the packet haven't been already received
    if(!this->is_duplicated(packet->id))
    {
        // Add to received packets
        this->received_packet_ids->push_back(packet->id);

        // if the packet has broadcast enabled or if dst is not for the current node
        if (packet->broadcast || packet->final_dst != this->get_my_node_name())
        {
            this->redirect(packet);
        }

        // if its a broadcast, then the msg has to be delivered to everyone (including us)
        // or if the final dst is equal to our name, then we are the final_dst.
        if (packet->broadcast || packet->final_dst == this->get_my_node_name())
        {
            this->mp->put_received_packet(std::move(packet));
        }
    }
    else 
    {
        XBT_INFO("Discarding packet %lu: duplicate", packet->id);
    }
}

void RingNetworkManager::broadcast(shared_ptr<Packet> packet) 
{
    uint16_t res = 0;

    // Apply filter function and send to left node
    if ((*packet->filter)(&this->left_node))
    {
        packet->dst = this->left_node.name;
        this->send_async(packet);
        res++;
    }

    // Apply filter function and send to right node
    if ((*packet->filter)(&this->right_node))
    {
        packet->dst = this->right_node.name;
        this->send_async(packet);
        res++;
    }
}

bool RingNetworkManager::is_duplicated(const packet_id id)
{
    auto r = this->received_packet_ids;

    // Try to find the id on the list
    return find(r->begin(), r->end(), id) != r->end();
}

void RingNetworkManager::redirect(unique_ptr<Packet> &p)
{
    auto new_p = make_shared<Packet>(*p);

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

    new_p->dst = dst;
    
    // Note that the final dst stays the same.
    // We specify is_redirected to true to prevent the orginal src from being overwritten.
    this->send_async(new_p, true);
}