#ifndef FALAFELS_MEDIATOR_HPP
#define FALAFELS_MEDIATOR_HPP

#include <format>
// #include <simgrid/s4u/MessageQueue.hpp>
// Temporary work around: using mailbox instead of messageQueues because get_async() is bug on this last
#include <simgrid/s4u/Mailbox.hpp>
#include <simgrid/s4u/ActivitySet.hpp>
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

    void wait_all_async_comms()
    {
        this->async_messages->wait_all();
    }
protected:
    /** 
     * Initialize queues to enable communication between Role and NetworkManager.
     * The format for MessageQueue name is the following: `<node name>_mq_<message queue name>`.
     */
    Mediator(node_name name)
    {
        this->mq_received_packets = simgrid::s4u::Mailbox::by_name(std::format("{}_mq_rp", name));
        this->mq_to_be_sent_packets = simgrid::s4u::Mailbox::by_name(std::format("{}_mq_tbsp", name));
        this->mq_nm_events = simgrid::s4u::Mailbox::by_name(std::format("{}_mq_nme", name));

        this->async_messages = new simgrid::s4u::ActivitySet();
    } 
    
    /** MessageQueue of packets received through the network */
    simgrid::s4u::Mailbox *mq_received_packets;

    /** MessageQueue of packets to send via the network */
    simgrid::s4u::Mailbox *mq_to_be_sent_packets;

    /** MessageQueue of events produced by a NetworkManager */
    simgrid::s4u::Mailbox *mq_nm_events;

    simgrid::s4u::ActivitySet *async_messages;
};

#endif // !FALAFELS_MEDIATOR_HPP
