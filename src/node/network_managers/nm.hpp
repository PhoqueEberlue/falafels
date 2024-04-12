/* Network Manager */
#ifndef FALAFELS_NETWORK_MANAGER_HPP
#define FALAFELS_NETWORK_MANAGER_HPP

#include <cstdint>
#include <functional>
#include <simgrid/s4u/Mailbox.hpp>
#include <vector>
#include "../../protocol.hpp"

using FilterNode = std::function<bool(NodeInfo*)>;

class NetworkManager 
{
protected:
    simgrid::s4u::Mailbox *mailbox;
    std::vector<NodeInfo*> *bootstrap_nodes;
    node_name my_node_name;

public:
    NetworkManager(){}
    virtual ~NetworkManager(){}
    virtual uint16_t broadcast(Packet*, FilterNode) = 0;
    virtual uint16_t broadcast_timeout(Packet*, FilterNode, uint64_t) = 0;
    virtual void send(Packet*, node_name) = 0;
    virtual bool send_timeout(Packet*, node_name, uint64_t) = 0;

    virtual Packet *get() = 0;
    virtual void set_bootstrap_nodes(std::vector<NodeInfo*> *nodes) = 0;

    std::vector<NodeInfo*> *get_bootstrap_nodes() { return this->bootstrap_nodes; }
    node_name get_my_node_name() { return this->my_node_name; }
};

namespace Filters {
    static bool trainers(NodeInfo *node_info)
    {
        return node_info->role == NodeRole::Trainer;
    }
}


#endif // !FALAFELS_NETWORK_MANAGER_HPP
