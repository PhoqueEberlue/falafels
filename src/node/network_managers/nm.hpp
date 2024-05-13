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

class NetworkManager 
{
public:
    struct NodeConnected
    {
    };

    /** Event thrown when our node is acting as a bootstrap node, indicating that all nodes were connected to us */
    struct ClusterConnected
    {
        uint16_t number_client_connected;
    };

    using Event = std::variant<NodeConnected, ClusterConnected>;
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

    /** Single get communication activity that listens incoming communications */
    simgrid::s4u::CommPtr pending_async_get;

    /** Optionally get packet to be sent if some */
    std::optional<std::shared_ptr<Packet>> get_to_be_sent_packet();

    /** Put a packet received by the network to the received packets */
    void put_received_packet(std::unique_ptr<Packet> packet);

    /** Put a new event */
    void put_nm_event(Event e);
public:  
    NetworkManager(NodeInfo node_info);
    virtual ~NetworkManager();

    /** Run one step of the NetworkManager */
    bool run();   

    /** Get NetworkManager's NodeInfo */
    NodeInfo get_my_node_info() { return this->my_node_info; }

    /** Utility to get my node name quicker */
    node_name get_my_node_name() { return this->my_node_info.name; }

    /** Set bootstrap_nodes */
    void set_bootstrap_nodes(std::vector<NodeInfo> *nodes);

    /** Set queues to communicate with the Role */
    void set_queues(std::shared_ptr<std::queue<std::unique_ptr<Packet>>> received, 
                    std::shared_ptr<std::queue<std::shared_ptr<Packet>>> to_be_sent,
                    std::shared_ptr<std::queue<std::unique_ptr<Event>>> nm_events);

    /** Wrapper function that either sends as broadcast or normally */
    void send_packet(std::shared_ptr<Packet> p);

    /** Classic send from the current node to another one. If is_redirected is set to true, the original source wont be overwritten */
    void send_async(std::shared_ptr<Packet> p, bool is_redirected=false);

    /* --------- Methods to be redefined by children classes --------- */

    /** Handle the registration regquests by creating the network links and sending confirmations to the connected nodes */
    virtual void handle_registration_requests() = 0;

    /** Send a registration request to one of our bootstrap node */
    virtual void send_registration_request() = 0;

    /** Handle a registration confirmation by connecting to the received list of node */
    virtual void handle_registration_confirmation(const Packet::RegistrationConfirmation &confirmation) = 0;

    /** Broadcast a packet to the node of our cluster matching the filter contained in the packet */
    virtual void broadcast(std::shared_ptr<Packet>) = 0;

    /** This function decides wether it should give the packet to the (local) Role (by sending it through received_packets queue)
        and/or if it should redirect it to another Node. */
    virtual void route_packet(std::unique_ptr<Packet> packet) = 0;
    /* --------------------------------------------------------------- */
private:
    /** Simgrid mailbox associated to the NetworkManager */
    simgrid::s4u::Mailbox *mailbox;

    /** Queue of the received packets that the role will be able to retrieve */
    std::shared_ptr<std::queue<std::unique_ptr<Packet>>> received_packets;

    /** Queue of the packets to be sent by the NetworkManager */
    std::shared_ptr<std::queue<std::shared_ptr<Packet>>> to_be_sent_packets;

    /** Queue of events that the NetworkManager can create */
    std::shared_ptr<std::queue<std::unique_ptr<Event>>> nm_events;

    /** Try to get a Packet from the Network, optionally returning a packet */
    std::optional<std::unique_ptr<Packet>> try_get();
};

#endif // !FALAFELS_NETWORK_MANAGER_HPP
