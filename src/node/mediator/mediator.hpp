#ifndef FALAFELS_MEDIATOR_HPP
#define FALAFELS_MEDIATOR_HPP

#include <memory>
#include <queue>
#include <variant>
#include "../../protocol.hpp"

class Mediator
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

    /** Set queues to enable communication between Role and NetworkManager */
    Mediator(std::shared_ptr<std::queue<std::unique_ptr<Packet>>> received, 
                    std::shared_ptr<std::queue<std::shared_ptr<Packet>>> to_be_sent,
                    std::shared_ptr<std::queue<std::unique_ptr<Event>>> nm_events)
    {
        this->received_packets = received;
        this->to_be_sent_packets = to_be_sent;
        this->nm_events = nm_events;
    }
protected:
    /** Queue of packets received through the network, that the Role can get */
    std::shared_ptr<std::queue<std::unique_ptr<Packet>>> received_packets;

    /** Queue of packets that the Role wants to send via the network */
    std::shared_ptr<std::queue<std::shared_ptr<Packet>>> to_be_sent_packets;

    /** Queue of NetworkManager events */
    std::shared_ptr<std::queue<std::unique_ptr<Event>>> nm_events;
};

#endif // !FALAFELS_MEDIATOR_HPP
