#include "centralized_nm.hpp"
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <simgrid/s4u/Mailbox.hpp>
#include <vector>
#include <xbt/log.h>

XBT_LOG_NEW_DEFAULT_CATEGORY(s4u_centralized_nm, "Messages specific for this example");

CentralizedNetworkManager::CentralizedNetworkManager(node_name name, NodeRole role)
{
    this->my_node_name = name;
    this->my_node_role = role;
    // Initializing our mailbox
    this->mailbox = simgrid::s4u::Mailbox::by_name(name);
}

CentralizedNetworkManager::~CentralizedNetworkManager() {};

void CentralizedNetworkManager::set_bootstrap_nodes(std::vector<NodeInfo*> *nodes)
{
    this->bootstrap_nodes = nodes;
};

uint16_t CentralizedNetworkManager::broadcast(Packet *packet, FilterNode filter)
{
    for(auto node_info : *this->get_bootstrap_nodes())
    {
        if (filter(node_info))
        {
            this->send(packet, node_info->name);
        }
    }

    // Delete our own reference of the packet
    // packet->decr_ref_count();

    return this->get_bootstrap_nodes()->size();
}

uint16_t CentralizedNetworkManager::broadcast(Packet *packet, FilterNode filter, uint64_t timeout)
{
    uint16_t nb_sent = 0;

    for(auto node_info : *this->get_bootstrap_nodes())
    {
        if (filter(node_info))
        {
            if (send(packet, node_info->name, timeout))
                nb_sent++;
        }
    }

    // Delete our own reference of the packet
    // packet->decr_ref_count();

    return nb_sent;
}

Packet *CentralizedNetworkManager::get()
{
    auto p = this->mailbox->get<Packet>();
    XBT_INFO("%s <--%s--- %s", this->my_node_name.c_str(), p->op_string.c_str(), p->src.c_str());
    return p;
}
