/* Network Manager */
#ifndef FALAFELS_S_NETWORK_MANAGER_HPP
#define FALAFELS_S_NETWORK_MANAGER_HPP

#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <queue>
#include <simgrid/forward.h>
#include <simgrid/s4u/Activity.hpp>
#include <simgrid/s4u/Mailbox.hpp>
#include <tuple>
#include <unordered_map>
#include <variant>
#include <vector>
#include "../../../../dml/include/protocol.hpp"
#include "../../../../dml/include/i_network_manager.hpp"
#include "../mediator/s_mediator_producer.hpp"

class SNetworkManager : INetworkManager
{ 
protected: 
    std::unique_ptr<SMediatorProducer> mp;

    /** Vector of nodes to connect to when launching the SNetworkManager. Used by Trainers that knows in advance 
        one of the aggregators */
    std::vector<protocol::NodeInfo> *bootstrap_nodes;

    /** NodeInfo of the Node controlling the SNetworkManager */
    protocol::NodeInfo my_node_info;

    /** Save of the registration requests to be used during the WAITING_REGISTRATION_REQUEST state */
    std::vector<protocol::operations::RegistrationRequest> *registration_requests;

    /** AcitivitySet for all put communications made by our node */
    simgrid::s4u::ActivitySet *pending_async_put;

    /** AcitivitySet regrouping communications (from others SNetworkManager) and messages (from our Role) */
    simgrid::s4u::ActivitySet *pending_comm_and_mess_get;
public:  
    SNetworkManager(protocol::NodeInfo node_info);
    virtual ~SNetworkManager();

    void set_mediator_producer(std::unique_ptr<SMediatorProducer> mp) { this->mp = std::move(mp); };

    /** Get SNetworkManager's NodeInfo */
    protocol::NodeInfo get_my_node_info() { return this->my_node_info; }

    /** Utility to get my node name quicker */
    protocol::node_name get_my_node_name() { return this->my_node_info.name; }

    /** Set bootstrap_nodes */
    void set_bootstrap_nodes(std::vector<protocol::NodeInfo> *nodes);
 
    /** Classic send from the current node to another one. If is_redirected is set to true, the original source wont be overwritten */
    void send_async(const std::unique_ptr<protocol::Packet> &p, bool is_redirected=false);

    void kill_role_actor();

    /** Blocking get a Packet from the Network */
    std::unique_ptr<protocol::Packet> get(const std::optional<double> timeout=std::nullopt);

    /** Async get a Packet from the Network */
    simgrid::s4u::CommPtr get_async();

    void init_run_activities();

    void clear_async_puts();
private:
    /** Simgrid mailbox associated to the SNetworkManager */
    simgrid::s4u::Mailbox *mailbox; 
};

#endif // !FALAFELS_S_NETWORK_MANAGER_HPP
