#include "centralized_nm.hpp"
#include <cstdint>
#include <cstdlib>
#include <cstring>
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
    packet->decr_ref_count();

    return this->get_bootstrap_nodes()->size();
}

uint16_t CentralizedNetworkManager::broadcast_timeout(Packet *packet, FilterNode filter, uint64_t timeout)
{
    uint16_t nb_sent = 0;

    for(auto node_info : *this->get_bootstrap_nodes())
    {
        if (filter(node_info))
        {
            // Would be better with shared pointers... Because right now each receiver will try to free the memory,
            // so we have to make duplicates
            if (send_timeout(packet, node_info->name, timeout))
                nb_sent++;
        }
    }

    // Delete our own reference of the packet
    packet->decr_ref_count();

    return nb_sent;
}

void CentralizedNetworkManager::send(Packet *packet, node_name name)
{
    auto receiver_mailbox = simgrid::s4u::Mailbox::by_name(name);
    packet->incr_ref_count();
    receiver_mailbox->put(packet, packet->get_packet_size());
}

bool CentralizedNetworkManager::send_timeout(Packet *packet, node_name name, uint64_t timeout)
{
    try 
    {
        auto receiver_mailbox = simgrid::s4u::Mailbox::by_name(name);
        packet->incr_ref_count();
        receiver_mailbox->put(packet, packet->get_packet_size(), timeout);
        return true;
    } 
    catch (simgrid::TimeoutException) 
    {
        packet->decr_ref_count();
        XBT_INFO("Timeout, couldn't send message from %s to %s", this->get_my_node_name().c_str(), name.c_str());
        return false;
    }
}

Packet *CentralizedNetworkManager::get()
{
    return this->mailbox->get<Packet>();
}
