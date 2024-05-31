#ifndef FALAFELS_MEDIATOR_HPP
#define FALAFELS_MEDIATOR_HPP

#include <memory>
#include <queue>
#include <variant>
#include "../../protocol.hpp"

/** 
 * Mediator enables communication between NetworkManagers and Roles.
 * Its constructor is protected to ensure the user only uses MediatorProducer and MediatorConsumer. 
 */
class Mediator
{
public: 
    /** Event thrown when our node has connected to the bootstrap node */
    struct NodeConnected {};

    /** Event thrown when our node is acting as a bootstrap node, indicating that all nodes were connected to us */
    struct ClusterConnected
    {
        uint16_t number_client_connected;
    };

    using Event = std::variant<NodeConnected, ClusterConnected>; 
protected:
    /** 
     * Set queues to enable communication between Role and NetworkManager. Note that obviously they should be the same
     * shared_ptr between the Consumer and Producer.
     */
    Mediator(std::shared_ptr<std::queue<std::unique_ptr<Packet>>> received, 
                    std::shared_ptr<std::queue<std::shared_ptr<Packet>>> to_be_sent,
                    std::shared_ptr<std::queue<std::unique_ptr<Event>>> nm_events)
    {
        this->received_packets = received;
        this->to_be_sent_packets = to_be_sent;
        this->nm_events = nm_events;
    }

    /** Queue of packets received through the network */
    std::shared_ptr<std::queue<std::unique_ptr<Packet>>> received_packets;

    /** Queue of packets to send via the network */
    std::shared_ptr<std::queue<std::shared_ptr<Packet>>> to_be_sent_packets;

    /** Queue of events produced by a NetworkManager */
    std::shared_ptr<std::queue<std::unique_ptr<Event>>> nm_events;
};

#endif // !FALAFELS_MEDIATOR_HPP
