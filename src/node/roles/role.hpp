/* Role */
#ifndef FALAFELS_ROLE_HPP
#define FALAFELS_ROLE_HPP

#include "../network_managers/nm.hpp"
#include <memory>
#include <optional>
#include <queue>

/**
 * Abstract class that defines a Node behaviour in a Federated Learning system.
 * There are 3 different types of roles that inherits from Role class.
 * - Aggregator: Aggregates the local models into a global one.
 * - Trainer: Trains a local model.
 * - Proxy: Redirect communications between clusters (HDFL/DFL).
 */
class Role
{
private:
    /** Queue of packets received through the network, that the Role can get */
    std::shared_ptr<std::queue<std::unique_ptr<Packet>>> received_packets;

    /** Queue of packets that the Role wants to send via the network */
    std::shared_ptr<std::queue<std::shared_ptr<Packet>>> to_be_sent_packets;

    /** Queue of NetworkManager events */
    std::shared_ptr<std::queue<std::unique_ptr<NetworkManager::Event>>> nm_events;
public:
    Role(){}
    virtual ~Role(){}

    /** Set queues to enable communication between Role and NetworkManager */
    void set_queues(std::shared_ptr<std::queue<std::unique_ptr<Packet>>> received, 
                    std::shared_ptr<std::queue<std::shared_ptr<Packet>>> to_be_sent,
                    std::shared_ptr<std::queue<std::unique_ptr<NetworkManager::Event>>> nm_events);

    /** Optionally get a Packet if available */
    std::optional<std::unique_ptr<Packet>> get_received_packet();

    /** Put a packet to be sent by the NetWorkManager */
    void put_to_be_sent_packet(Packet packet);

    /** Optionally get a NMEvent if available */
    std::optional<std::unique_ptr<NetworkManager::Event>> get_nm_event();

    /* --- Functions to be implemented by the children classes --- */
    virtual void run() = 0;
    virtual NodeRole get_role_type() = 0;
    /* ----------------------------------------------------------- */
};

#endif // !FALAFELS_ROLE_HPP
