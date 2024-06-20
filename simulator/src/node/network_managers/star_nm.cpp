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

#include "star_nm.hpp"
#include "../../dot.hpp"
#include "nm.hpp"

using namespace std;

XBT_LOG_NEW_DEFAULT_CATEGORY(s4u_star_nm, "Messages specific for this example");

StarNetworkManager::StarNetworkManager(NodeInfo node_info) : NetworkManager(node_info)
{
    this->connected_nodes = new vector<NodeInfo>();
}

StarNetworkManager::~StarNetworkManager() 
{
    delete this->connected_nodes;
};

void StarNetworkManager::handle_registration_requests()
{
    xbt_assert(this->my_node_info.role == NodeRole::Aggregator);

    DOTGenerator::get_instance().add_to_cluster(
        std::format("cluster-{}", this->my_node_info.name),
        std::format("{} [label=\"{}\", color=green]", this->my_node_info.name, this->my_node_info.name)
    );

    for (auto request : *this->registration_requests)
    {
        this->connected_nodes->push_back(request.node_to_register); 

        DOTGenerator::get_instance().add_to_cluster(
            std::format("cluster-{}", this->my_node_info.name),
            std::format("{} [label=\"{}\", color=yellow]", request.node_to_register.name, request.node_to_register.name)
        );

        DOTGenerator::get_instance().add_to_cluster(
            std::format("cluster-{}", this->my_node_info.name),
            std::format("{} -> {} [color=green]", this->my_node_info.name, request.node_to_register.name)
        );

        auto node_list = vector<NodeInfo>();
        node_list.push_back(this->my_node_info);

        auto res_p = new Packet(
            request.node_to_register.name, request.node_to_register.name,
            Packet::RegistrationConfirmation(
                make_shared<vector<NodeInfo>>(node_list)
            )
        );

        this->send_async(res_p);
    }

    this->mp->put_nm_event(
        new Mediator::Event {
            Mediator::ClusterConnected { .number_client_connected=(uint16_t)this->connected_nodes->size() }
        }
    );
}

void StarNetworkManager::send_registration_request()
{
    // This assert is'nt true in the case of hierarchical aggregator...
    // xbt_assert(this->my_node_info.role == NodeRole::Trainer);

    // Take the first bootstrap node, in a centralized topology we should have only one anyways.
    auto bootstrap_node = this->bootstrap_nodes->at(0);

    auto p = new Packet(
        bootstrap_node.name, bootstrap_node.name,
        Packet::RegistrationRequest(
            this->my_node_info
        )
    );

    this->send_async(p);
}


void StarNetworkManager::handle_registration_confirmation(const Packet::RegistrationConfirmation &confirmation)
{
    for (auto node: *confirmation.node_list)
    {
        this->connected_nodes->push_back(node);
    }

    this->mp->put_nm_event(
        new Mediator::Event { Mediator::NodeConnected {} }
    );
}

void StarNetworkManager::broadcast(Packet *p)
{
    for(auto node_info : *this->connected_nodes)
    {
        // Apply the filter function and send if it returned true
        if ((*p->filter)(&node_info))
        {
            p->dst = node_info.name;
            this->send_async(p);
        }
    }
}

void StarNetworkManager::route_packet(Packet *p)
{
    // In star_nm we route everything to the Role.
    this->mp->put_received_packet(p);
}
