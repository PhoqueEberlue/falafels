/* Network Manager */
#ifndef FALAFELS_NETWORK_MANAGER_HPP
#define FALAFELS_NETWORK_MANAGER_HPP

#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <simgrid/forward.h>
#include <simgrid/s4u/Mailbox.hpp>
#include <tuple>
#include <unordered_map>
#include <vector>
#include "../../protocol.hpp"

using FilterNode = std::function<bool(NodeInfo*)>;

class NetworkManager 
{
protected:
    simgrid::s4u::Mailbox *mailbox;
    std::vector<NodeInfo> *bootstrap_nodes;
    NodeInfo my_node_info;

    /** get to be used for inherited classes of NetworkManager */
    std::unique_ptr<Packet> get(const std::optional<double> &timeout=std::nullopt);
public:
    NetworkManager();

    void send(std::shared_ptr<Packet>, node_name dst, const std::optional<double> &timeout=std::nullopt);

    void send_async(std::shared_ptr<Packet>, node_name);
    void wait_last_comms(const std::optional<double> &timeout=std::nullopt);

    std::vector<NodeInfo> *get_bootstrap_nodes() { return this->bootstrap_nodes; }
    node_name get_my_node_name() { return this->my_node_info.name; }
    NodeInfo get_my_node_info() { return this->my_node_info; }
    void set_bootstrap_nodes(std::vector<NodeInfo> *nodes);

    /* --------- Methods to be redefined by children classes --------- */
    virtual ~NetworkManager();
    virtual uint16_t handle_registration_requests() = 0;
    virtual void send_registration_request() = 0;
    virtual void broadcast(std::shared_ptr<Packet>, FilterNode, const std::optional<double> &timeout=std::nullopt) = 0;

    /** get to be used by roles that will be override by the specific NetworkManager */
    virtual std::unique_ptr<Packet> get_packet(const std::optional<double> &timeout=std::nullopt) = 0;
private:
    simgrid::s4u::ActivitySet *pending_async_comms;
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
