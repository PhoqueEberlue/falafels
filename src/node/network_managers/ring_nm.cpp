#include <cstdint>
#include <format>
#include <memory>
#include <simgrid/Exception.hpp>
#include <simgrid/forward.h>
#include <variant>
#include <vector>
#include <xbt/asserts.h>
#include <xbt/ex.h>
#include <xbt/log.h>
#include <simgrid/s4u/Engine.hpp>

#include "ring_nm.hpp"
#include "../../dot.hpp"

using namespace std;

XBT_LOG_NEW_DEFAULT_CATEGORY(s4u_ring_nm, "Messages specific for this example");

RingNetworkManager::RingNetworkManager(NodeInfo node_info)
{ 
    this->my_node_info = node_info;

    // Initializing our mailbox
    this->mailbox = simgrid::s4u::Mailbox::by_name(node_info.name);

    this->received_packets = new vector<packet_id>();
}

RingNetworkManager::~RingNetworkManager()
{
    delete this->received_packets;
};

uint16_t RingNetworkManager::handle_registration_requests()
{
    xbt_assert(this->my_node_info.role == NodeRole::Aggregator);
 
    DOTGenerator::get_instance().add_to_cluster(
        std::format("cluster-{}", this->my_node_info.name),
        std::format("{} [label=\"{}\", color=green]", this->my_node_info.name, this->my_node_info.name)
    );

    auto node_list_tmp = vector<NodeInfo>();
    
    unique_ptr<Packet> p;
 
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

        if (auto *reg_req = get_if<Packet::RegistrationRequest>(&p->op))
        {
            node_list_tmp.push_back(reg_req->node_to_register); 
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

        auto neigbours = vector<NodeInfo>();
        neigbours.push_back(left_n);
        neigbours.push_back(right_n);

        auto res_p = make_shared<Packet>(Packet(
            this->get_my_node_name(), 
            node_list_tmp.at(i).name, // Sent the packt to the current node
            Packet::RegistrationConfirmation(
                make_shared<vector<NodeInfo>>(neigbours)
            )
        ));

        this->send(res_p, node_list_tmp.at(i).name);
    } 

    DOTGenerator::get_instance().add_to_cluster(
        std::format("cluster-{}", this->my_node_info.name),
        std::format("{} -> {} [color=green]", this->my_node_info.name, this->left_node.name)
    );

    DOTGenerator::get_instance().add_to_cluster(
        std::format("cluster-{}", this->my_node_info.name),
        std::format("{} -> {} [color=green]", this->my_node_info.name, this->right_node.name)
    );

    // Return the number of nodes that have been registered
    return node_list_tmp.size();
}

void RingNetworkManager::send_registration_request()
{
    xbt_assert(this->my_node_info.role == NodeRole::Trainer);

    // Take the first bootstrap node. TODO: handle multiple bootstrap nodes??
    auto bootstrap_node = this->bootstrap_nodes->at(0);

    auto p = make_shared<Packet>(Packet(
        this->get_my_node_name(), bootstrap_node.name, 
        Packet::RegistrationRequest(
            this->my_node_info
        )
    ));

    // Send the request
    this->send(p, bootstrap_node.name);

    unique_ptr<Packet> response;

    // Wait for confirmation 
    while (true) 
    {
        response = this->get();

        if (auto *reg_conf = get_if<Packet::RegistrationConfirmation>(&response->op))
        {
            XBT_INFO("Succesfully registered");
            this->left_node = reg_conf->node_list->at(0);
            this->right_node = reg_conf->node_list->at(1);

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
           
            break;
        }
    }
}

void RingNetworkManager::broadcast(shared_ptr<Packet> packet, FilterNode filter, const optional<double> &timeout) 
{
    uint16_t res = 0;

    // Send to left node
    if (filter(&this->left_node))
    {
        this->send(packet, this->left_node.name, timeout);
        res++;
    }

    // Send to right node
    if (filter(&this->right_node))
    {
        this->send(packet, this->right_node.name, timeout);
        res++;
    }
}

bool RingNetworkManager::is_duplicated(std::unique_ptr<Packet> &packet)
{
    auto r = this->received_packets;
    return find(r->begin(), r->end(), packet->id) != r->end();
}

unique_ptr<Packet> RingNetworkManager::get_packet(const optional<double> &timeout)
{
    unique_ptr<Packet> p;
    bool cond = true;
 
    while (cond)
    {
        p = this->get(timeout);
 
        // Check that the packet haven't been already received
        if(!this->is_duplicated(p))
        {
            // if the packet has broadcast enabled or if dst is not for the current node
            if (p->final_dst == "BROADCAST" || p->final_dst != this->get_my_node_name())
            {
                this->redirect(p);
            }

            // Add to received packets
            this->received_packets->push_back(p->id);
            cond = false;
        }
        else 
        {
            XBT_INFO("Discarding packet %lu: duplicate", p->id);
        }
    }
 
    return p;
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
    
    // Note that the final dest stays the same.
    this->send_async(new_p, dst);
}
