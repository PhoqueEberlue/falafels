/* Network Manager */
#ifndef FALAFELS_NETWORK_MANAGER_HPP
#define FALAFELS_NETWORK_MANAGER_HPP

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
#include "../../protocol.hpp"
#include "../mediator/mediator_producer.hpp"

class NetworkManager 
{ 
protected: 
    using State = enum
    {
        INITIALIZING,
        WAITING_REGISTRATION_REQUEST,
        WAITING_REGISTRATION_CONFIRMATION,
        RUNNING,
        KILLING,
    };

    /** State of the NetworkManager */
    State state = INITIALIZING;

    std::unique_ptr<MediatorProducer> mp;

    /** Vector of nodes to connect to when launching the NetworkManager. Used by Trainers that knows in advance 
        one of the aggregators */
    std::vector<protocol::NodeInfo> *bootstrap_nodes;

    /** NodeInfo of the Node controlling the NetworkManager */
    protocol::NodeInfo my_node_info;

    /** Save of the registration requests to be used during the WAITING_REGISTRATION_REQUEST state */
    std::vector<protocol::operations::RegistrationRequest> *registration_requests;

    /** Save of the start time of the aggregator */
    std::optional<double> start_time;

    /** AcitivitySet for all put communications made by our node */
    simgrid::s4u::ActivitySet *pending_async_put;

    /** AcitivitySet regrouping communications (from others NetworkManager) and messages (from our Role) */
    simgrid::s4u::ActivitySet *pending_comm_and_mess_get;
public:  
    NetworkManager(protocol::NodeInfo node_info);
    virtual ~NetworkManager();

    void set_mediator_producer(std::unique_ptr<MediatorProducer> mp) { this->mp = std::move(mp); };

    /** Get NetworkManager's NodeInfo */
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

    void if_target_put_op(std::unique_ptr<protocol::Packet> p);

    /* --------- Methods to be redefined by children classes --------- */
    /** Run the main execution function of the NetworkManager */
    virtual void run() = 0;

    /** Handle the registration regquests by creating the network links and sending confirmations to the connected nodes */
    virtual void handle_registration_requests() = 0;

    /** Send a registration request to one of our bootstrap node */
    virtual void send_registration_request() = 0;

    /** Handle a registration confirmation by connecting to the received list of node */
    virtual void handle_registration_confirmation(const protocol::operations::RegistrationConfirmation &confirmation) = 0;

    /** Broadcast a packet to the node of our cluster matching the filter contained in the packet */
    virtual void broadcast(const std::unique_ptr<protocol::Packet> &p, bool is_redirected=false) = 0;

    /** This function decides wether it should give the packet to the (local) Role (by sending it through received_packets queue)
        and/or if it should redirect it to another Node. */
    // virtual void route_packet(std::unique_ptr<protocol::Packet> p) = 0;

    virtual void handle_kill_phase() = 0;
    /* --------------------------------------------------------------- */
private:
    /** Simgrid mailbox associated to the NetworkManager */
    simgrid::s4u::Mailbox *mailbox; 
};

#endif // !FALAFELS_NETWORK_MANAGER_HPP
