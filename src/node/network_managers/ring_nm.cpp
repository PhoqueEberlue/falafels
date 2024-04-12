#include <cstdint>
#include <simgrid/Exception.hpp>
#include <vector>
#include <xbt/asserts.h>
#include <xbt/ex.h>
#include "ring_nm.hpp"

XBT_LOG_NEW_DEFAULT_CATEGORY(s4u_ring_nm, "Messages specific for this example");

RingNetworkManager::RingNetworkManager(node_name name)
{
    this->my_node_name = name;
    // Initializing our mailbox
    this->mailbox = simgrid::s4u::Mailbox::by_name(name);
}

RingNetworkManager::~RingNetworkManager() {};

void RingNetworkManager::set_bootstrap_nodes(std::vector<NodeInfo*> *nodes)
{
    xbt_assert(this->bootstrap_nodes->size() != 2, "The RingNetworkManager must have at least 2 bootstrap nodes");
    this->left_node = nodes->at(0);
    this->right_node = nodes->at(1);
}

uint16_t RingNetworkManager::broadcast(Packet *packet, FilterNode filter) {
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
    packet->decr_ref_count();

    return res;
}

uint16_t RingNetworkManager::broadcast_timeout(Packet *packet, FilterNode filter, uint64_t timeout)
{
    // TODO
}
