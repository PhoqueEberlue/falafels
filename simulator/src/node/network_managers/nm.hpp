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
    std::vector<NodeInfo> *bootstrap_nodes;

    /** NodeInfo of the Node controlling the NetworkManager */
    const NodeInfo my_node_info;

    /** Save of the registration requests to be used during the WAITING_REGISTRATION_REQUEST state */
    std::vector<Packet::RegistrationRequest> *registration_requests;

    /** Save of the start time of the aggregator */
    std::optional<double> start_time;

    /** AcitivitySet for all put communications made by our node */
    simgrid::s4u::ActivitySet *pending_async_put;

    /** AcitivitySet regrouping communications and messages */
    simgrid::s4u::ActivitySet *pending_comm_and_mess_get;
public:  
    NetworkManager(NodeInfo node_info);
    virtual ~NetworkManager();

    /** Run one step of the NetworkManager */
    void run();   

    void set_mediator_producer(std::unique_ptr<MediatorProducer> mp) { this->mp = std::move(mp); };

    /** Get NetworkManager's NodeInfo */
    NodeInfo get_my_node_info() { return this->my_node_info; }

    /** Utility to get my node name quicker */
    node_name get_my_node_name() { return this->my_node_info.name; }

    /** Set bootstrap_nodes */
    void set_bootstrap_nodes(std::vector<NodeInfo> *nodes);
 
    /** Wrapper function that either sends as broadcast or normally */
    void send_packet(Packet *p);

    /** Classic send from the current node to another one. If is_redirected is set to true, the original source wont be overwritten */
    void send_async(Packet *p, bool is_redirected=false);

    /* --------- Methods to be redefined by children classes --------- */

    /** Handle the registration regquests by creating the network links and sending confirmations to the connected nodes */
    virtual void handle_registration_requests() = 0;

    /** Send a registration request to one of our bootstrap node */
    virtual void send_registration_request() = 0;

    /** Handle a registration confirmation by connecting to the received list of node */
    virtual void handle_registration_confirmation(const Packet::RegistrationConfirmation &confirmation) = 0;

    /** Broadcast a packet to the node of our cluster matching the filter contained in the packet */
    virtual void broadcast(Packet *p) = 0;

    /** This function decides wether it should give the packet to the (local) Role (by sending it through received_packets queue)
        and/or if it should redirect it to another Node. */
    virtual void route_packet(Packet *p) = 0;
    /* --------------------------------------------------------------- */
private:
    /** Simgrid mailbox associated to the NetworkManager */
    simgrid::s4u::Mailbox *mailbox;

    /** Blocking get a Packet from the Network */
    std::unique_ptr<Packet> get(const std::optional<double> timeout=std::nullopt);

    /** Async get a Packet from the Network */
    simgrid::s4u::CommPtr get_async();

    Packet *to_be_sent_res;
};

#endif // !FALAFELS_NETWORK_MANAGER_HPP
