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

using namespace std;

XBT_LOG_NEW_DEFAULT_CATEGORY(s4u_star_nm, "Messages specific for this example");

StarNetworkManager::StarNetworkManager(NodeInfo node_info)
{
    this->my_node_info = node_info;
    // Initializing our mailbox
    this->mailbox = simgrid::s4u::Mailbox::by_name(this->my_node_info.name);
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

        auto res_p = make_shared<Packet>(Packet(
            this->get_my_node_name(), request.node_to_register.name,
            Packet::RegistrationConfirmation(
                make_shared<vector<NodeInfo>>(node_list)
            )
        ));

        this->send_async(res_p);
    }
}

void StarNetworkManager::send_registration_request()
{
    // This assert is'nt true in the case of hierarchical aggregator...
    // xbt_assert(this->my_node_info.role == NodeRole::Trainer);

    // Take the first bootstrap node, in a centralized topology we should have only one anyways.
    auto bootstrap_node = this->bootstrap_nodes->at(0);

    auto p = make_shared<Packet>(Packet(
        this->get_my_node_name(), bootstrap_node.name,
        Packet::RegistrationRequest(
            this->my_node_info
        )
    ));

    this->send_async(p);
}


void StarNetworkManager::handle_registration_confirmation(const Packet::RegistrationConfirmation &confirmation)
{
    XBT_INFO("node list : %s", confirmation.node_list->at(0).name.c_str());

    for (auto node: *confirmation.node_list)
    {
        this->connected_nodes->push_back(node);
    }
}

void StarNetworkManager::broadcast(shared_ptr<Packet> packet)
{
    for(auto node_info : *this->connected_nodes)
    {
        if ((*packet->filter)(&node_info))
        {
            this->send_async(packet);
        }
    }
}

/**  
 * In star_nm we route everything to the Role.
 */
void StarNetworkManager::route_packet(unique_ptr<Packet> packet)
{
    this->put_received_packet(std::move(packet));
}
