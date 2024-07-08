#ifndef FALAFELS_S_MEDIATOR_HPP
#define FALAFELS_S_MEDIATOR_HPP

#include <format>
#include <simgrid/forward.h>
#include <simgrid/s4u/MessageQueue.hpp>
#include <simgrid/s4u/ActivitySet.hpp>
#include <variant>
#include "../../../../dml/include/protocol.hpp"

/** 
 * SimulatedMediator enables communication between NetworkManagers and Roles.
 * Its constructor is protected to ensure the user only uses SimulatedMediatorProducer and SimulatedMediatorConsumer. 
 */
class SimulatedMediator
{ 
protected:
    /** 
     * Initialize queues to enable communication between Role and NetworkManager.
     * The format for MessageQueue name is the following: `<node name>_mq_<message queue name>`.
     */
    SimulatedMediator(protocol::node_name name)
    {
        this->mq_received_operations = simgrid::s4u::MessageQueue::by_name(std::format("{}_mq_rp", name));
        this->mq_to_be_sent_packets = simgrid::s4u::MessageQueue::by_name(std::format("{}_mq_tbsp", name));
        this->mq_nm_events = simgrid::s4u::MessageQueue::by_name(std::format("{}_mq_nme", name));

        this->async_messages = new simgrid::s4u::ActivitySet();
    } 

    ~SimulatedMediator() { delete this->async_messages; };
    
    /** MessageQueue of packets received through the network */
    simgrid::s4u::MessageQueue *mq_received_operations;

    /** MessageQueue of packets to send via the network */
    simgrid::s4u::MessageQueue *mq_to_be_sent_packets;

    /** MessageQueue of events produced by a NetworkManager */
    simgrid::s4u::MessageQueue *mq_nm_events;

    simgrid::s4u::ActivitySet *async_messages;
};

#endif // !FALAFELS_S_MEDIATOR_HPP
