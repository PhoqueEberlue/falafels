#include "star_nm.hpp"
#include <cstdint>
#include <cstdlib>
#include <cstring>
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
    delete this->bootstrap_nodes;
};

uint16_t StarNetworkManager::handle_registration_requests()
{
    xbt_assert(this->my_node_info.role == NodeRole::Aggregator);

    uint16_t number_registration = 0;
    unique_ptr<Packet> p;

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
            this->connected_nodes->push_back(reg_req->node_to_register);

            XBT_INFO("Added %s as a connected node", reg_req->node_to_register.name.c_str());

            auto node_list = vector<NodeInfo>();
            node_list.push_back(this->my_node_info);
            XBT_INFO("node list : %s", node_list.at(0).name.c_str());

            auto res_p = make_shared<Packet>(Packet(
                this->get_my_node_name(), p->final_dst,
                Packet::RegistrationConfirmation(
                    make_shared<vector<NodeInfo>>(node_list)
                )
            ));

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

    auto p = make_shared<Packet>(Packet(
        this->get_my_node_name(), bootstrap_node.name,
        Packet::RegistrationRequest(
            this->my_node_info
        )
    ));

    this->send(p, bootstrap_node.name);

    unique_ptr<Packet> response;

    while (true) 
    {
        response = this->get();

        if (auto *conf = get_if<Packet::RegistrationConfirmation>(&response->op))
        {
            XBT_INFO("node list : %s", conf->node_list->at(0).name.c_str());
            for (auto node: *conf->node_list)
            {
                this->connected_nodes->push_back(node);
                XBT_INFO("Add connection to %s", node.name.c_str());
            }

            break;
        }
    }
}

void StarNetworkManager::broadcast(shared_ptr<Packet> packet, FilterNode filter, const optional<double> &timeout)
{
    for(auto node_info : *this->connected_nodes)
    {
        if (filter(&node_info))
        {
            if (timeout)
            {
                this->send(packet, node_info.name, *timeout);
            }
            else
            {
                this->send(packet, node_info.name);
            }
        }
    }
}

unique_ptr<Packet> StarNetworkManager::get_packet(const optional<double> &timeout)
{
    // Here we don't need to do more stuff than calling base NetworkManager's get.
    return this->get(timeout);
}
