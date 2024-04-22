/* Network Manager */
#ifndef FALAFELS_NETWORK_MANAGER_HPP
#define FALAFELS_NETWORK_MANAGER_HPP

#include <cstdint>
#include <functional>
#include <simgrid/forward.h>
#include <simgrid/s4u/Mailbox.hpp>
#include <vector>
#include "../../protocol.hpp"

using FilterNode = std::function<bool(NodeInfo*)>;

class NetworkManager 
{
protected:
    simgrid::s4u::Mailbox *mailbox;
    std::vector<NodeInfo*> *bootstrap_nodes;
    NodeInfo my_node_info;

public:
    NetworkManager(){}
    void send(Packet*, node_name dst);
    bool send(Packet*, node_name dst, uint64_t time_out);
    simgrid::s4u::CommPtr send_async(Packet*, node_name);

    virtual ~NetworkManager(){}
    virtual std::unique_ptr<Packet> get() = 0;
    virtual std::unique_ptr<Packet> get(double timeout) = 0;

    virtual uint16_t handle_registration_requests() = 0;
    virtual void send_registration_request() = 0;
    virtual uint16_t broadcast(Packet*, FilterNode) = 0;
    virtual uint16_t broadcast(Packet*, FilterNode, uint64_t time_out) = 0;
    // virtual void wait_last_comms() = 0;

    std::vector<NodeInfo*> *get_bootstrap_nodes() { return this->bootstrap_nodes; }
    node_name get_my_node_name() { return this->my_node_info.name; }
    NodeInfo *get_my_node_info() { return &this->my_node_info; }
    void set_bootstrap_nodes(std::vector<NodeInfo*> *nodes);
};

namespace Filters {
    static bool trainers(NodeInfo *node_info)
    {
        return node_info->role == NodeRole::Trainer;
    }

    static bool trainers_and_aggregators(NodeInfo *node_info)
    {
        return node_info->role == NodeRole::Trainer || 
               node_info->role == NodeRole::Aggregator;
    }
}


#endif // !FALAFELS_NETWORK_MANAGER_HPP
