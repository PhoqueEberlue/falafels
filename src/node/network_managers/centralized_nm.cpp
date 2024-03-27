#include "centralized_nm.hpp"
#include <simgrid/s4u/Mailbox.hpp>
#include <simgrid/Exception.hpp>
#include <vector>
#include <xbt/log.h>

XBT_LOG_NEW_DEFAULT_CATEGORY(s4u_centralized_nm, "Messages specific for this example");

CentralizedNetworkManager::CentralizedNetworkManager(node_name name)
{
    this->my_node_name = name;
    // Initializing our mailbox
    this->mailbox = simgrid::s4u::Mailbox::by_name(name);
}

CentralizedNetworkManager::~CentralizedNetworkManager() {};

std::vector<node_name> CentralizedNetworkManager::get_node_names_filter(std::function<bool(NodeInfo*)>filter)
{
    std::vector<node_name> result = {};

    for(auto node_info : *this->get_bootstrap_nodes())
    {
        if (filter(node_info))
        {
            result.push_back(node_info->name);
        }
    }

    return result;
}

void CentralizedNetworkManager::put(Packet *packet, node_name name) {
    auto receiver_mailbox = simgrid::s4u::Mailbox::by_name(name);

    receiver_mailbox->put(packet, compute_packet_size(packet));
}

bool CentralizedNetworkManager::put_timeout(Packet *packet, node_name name, uint64_t timeout)
{
    auto receiver_mailbox = simgrid::s4u::Mailbox::by_name(name);

    try 
    {
        receiver_mailbox->put(packet, compute_packet_size(packet), timeout);
        return true;
    } 
    catch (simgrid::TimeoutException) 
    {
        XBT_INFO("Timeout, couldn't send message from %s to %s", this->get_my_node_name().c_str(), name.c_str());
        return false;
    }
}

Packet *CentralizedNetworkManager::get()
{
    return this->mailbox->get<Packet>();
}
