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

    State state = INITIALIZING;

    std::vector<NodeInfo> *bootstrap_nodes;
    NodeInfo my_node_info;

    std::vector<Packet::RegistrationRequest> *registration_requests;
    std::optional<double> time;

    /** get to be used for inherited classes of NetworkManager */
    // std::unique_ptr<Packet> get(const std::optional<double> &timeout=std::nullopt);

    std::optional<std::shared_ptr<Packet>> get_to_be_sent_packet();
    void put_received_packet(std::unique_ptr<Packet> packet);
    void put_nm_event(Event e);

    simgrid::s4u::ActivitySet *pending_async_put;
    simgrid::s4u::CommPtr pending_async_get;
public:  
    simgrid::s4u::Mailbox *mailbox;
    NetworkManager(NodeInfo node_info);
    bool run();

    uint16_t get_nb_connected_clients() { return this->registration_requests->size(); }

    void send_packet(std::shared_ptr<Packet> p);
    void send_async(std::shared_ptr<Packet> p, bool is_redirected=false);

    void wait_last_comms(const std::optional<double> &timeout=std::nullopt);

    std::vector<NodeInfo> *get_bootstrap_nodes() { return this->bootstrap_nodes; }
    node_name get_my_node_name() { return this->my_node_info.name; }
    NodeInfo get_my_node_info() { return this->my_node_info; }
    void set_bootstrap_nodes(std::vector<NodeInfo> *nodes);

    void set_queues(std::shared_ptr<std::queue<std::unique_ptr<Packet>>> received, 
                    std::shared_ptr<std::queue<std::shared_ptr<Packet>>> to_be_sent,
                    std::shared_ptr<std::queue<std::unique_ptr<Event>>> nm_events);

    /* --------- Methods to be redefined by children classes --------- */
    virtual ~NetworkManager();
    virtual void handle_registration_requests() = 0;
    virtual void send_registration_request() = 0;
    virtual void handle_registration_confirmation(const Packet::RegistrationConfirmation &confirmation) = 0;
    virtual void broadcast(std::shared_ptr<Packet>) = 0;

    /** This function decides wether it should give the packet to the (local) Role (by sending it through received_packets vector)
        or if it should redirect it to another Node. */
    virtual void route_packet(std::unique_ptr<Packet> packet) = 0;
private:

    std::shared_ptr<std::queue<std::unique_ptr<Packet>>> received_packets;
    std::shared_ptr<std::queue<std::shared_ptr<Packet>>> to_be_sent_packets;
    std::shared_ptr<std::queue<std::unique_ptr<Event>>> nm_events;
    std::optional<std::unique_ptr<Packet>> try_get();
};

#endif // !FALAFELS_NETWORK_MANAGER_HPP
