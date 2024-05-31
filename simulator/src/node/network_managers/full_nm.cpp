#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <format>
#include <iterator>
#include <memory>
#include <optional>
#include <ostream>
#include <simgrid/Exception.hpp>
#include <simgrid/s4u/Engine.hpp>
#include <simgrid/s4u/Mailbox.hpp>
#include <variant>
#include <vector>
#include <xbt/asserts.h>
#include <xbt/log.h>

#include "full_nm.hpp"
#include "../../dot.hpp"
#include "nm.hpp"

using namespace std;

XBT_LOG_NEW_DEFAULT_CATEGORY(s4u_full_nm, "Messages specific for this example");

FullyConnectedNetworkManager::FullyConnectedNetworkManager(NodeInfo node_info) : NetworkManager(node_info)
{
    this->connected_nodes = new vector<NodeInfo>();
}

FullyConnectedNetworkManager::~FullyConnectedNetworkManager() 
{
    delete this->connected_nodes;
};

void FullyConnectedNetworkManager::handle_registration_requests()
{
    xbt_assert(this->my_node_info.role == NodeRole::Aggregator);

    DOTGenerator::get_instance().add_to_cluster(
        std::format("cluster-{}", this->my_node_info.name),
        std::format("{} [label=\"{}\", color=green]", this->my_node_info.name, this->my_node_info.name)
    );

    // List containing all nodes of the network that will be sent back to all nodes
    auto nodes_to_connect = make_shared<vector<NodeInfo>>();
    nodes_to_connect->push_back(this->my_node_info);

    // First loop to add each node that made a request to the list
    for (auto request : *this->registration_requests)
    {
        // Add to local vector that saves connected nodes
        this->connected_nodes->push_back(request.node_to_register); 

        // Add to list passed as packet payload
        nodes_to_connect->push_back(request.node_to_register);
    }
    
    // Second loop to send confirmation to each nodes
    for (auto request : *this->registration_requests)
    {
        DOTGenerator::get_instance().add_to_cluster(
            std::format("cluster-{}", this->my_node_info.name),
            std::format("{} [label=\"{}\", color=yellow]", request.node_to_register.name, request.node_to_register.name)
        );

        DOTGenerator::get_instance().add_to_cluster(
            std::format("cluster-{}", this->my_node_info.name),
            std::format("{} -> {} [color=green]", this->my_node_info.name, request.node_to_register.name)
        );

        auto res_p = make_shared<Packet>(Packet(
            request.node_to_register.name, request.node_to_register.name,
            Packet::RegistrationConfirmation {
                // Here full list is sent, including the same node that we send the packet to
                .node_list=nodes_to_connect
            }
        ));

        this->send_async(res_p);
    }

    this->mp->put_nm_event(
        Mediator::ClusterConnected { .number_client_connected=(uint16_t)this->connected_nodes->size() }
    );
}

void FullyConnectedNetworkManager::send_registration_request()
{
    // This assert is'nt true in the case of hierarchical aggregator...
    // xbt_assert(this->my_node_info.role == NodeRole::Trainer);

    // Take the first bootstrap node, in a centralized topology we should have only one anyways.
    auto bootstrap_node = this->bootstrap_nodes->at(0);

    auto p = make_shared<Packet>(Packet(
        bootstrap_node.name, bootstrap_node.name,
        Packet::RegistrationRequest(
            this->my_node_info
        )
    ));

    this->send_async(p);
}


void FullyConnectedNetworkManager::handle_registration_confirmation(const Packet::RegistrationConfirmation &confirmation)
{
    for (auto node: *confirmation.node_list)
    {
        // Don't add node if its the same as the current one
        if (node.name != this->get_my_node_name())
            this->connected_nodes->push_back(node);
    }

    this->mp->put_nm_event(Mediator::NodeConnected {});
}

void FullyConnectedNetworkManager::broadcast(shared_ptr<Packet> packet)
{
    for(auto node_info : *this->connected_nodes)
    {
        // Apply the filter function and send if it returned true
        if ((*packet->filter)(&node_info))
        {
            packet->dst = node_info.name;
            this->send_async(packet);
        }
    }
}

void FullyConnectedNetworkManager::route_packet(unique_ptr<Packet> packet)
{
    // In full_nm we route everything to the Role.
    this->mp->put_received_packet(std::move(packet));
}
