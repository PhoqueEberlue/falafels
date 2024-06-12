#ifndef FALAFELS_MEDIATOR_PRODUCER_HPP
#define FALAFELS_MEDIATOR_PRODUCER_HPP


#include "mediator.hpp"
#include <boost/smart_ptr/intrusive_ptr.hpp>
#include <simgrid/forward.h>
#include <xbt/asserts.h>
#include <xbt/log.h>

using namespace std;

/**
 * MediatorProducer is the end to be used my Networkmanagers.
 * It can:
 * - Put received packets
 * - Put Network events
 * - Get packets to be sent
 */
class MediatorProducer : public Mediator
{
public:
    MediatorProducer(std::shared_ptr<std::queue<std::unique_ptr<Packet>>> received, 
                     std::shared_ptr<std::queue<std::shared_ptr<Packet>>> to_be_sent,
                     std::shared_ptr<std::queue<std::unique_ptr<Event>>> nm_events,
                     simgrid::s4u::ActivitySet *node_activities)
    : Mediator(received, to_be_sent, nm_events, node_activities) {}

    /** Optionally get packet to be sent if some */
    std::optional<std::shared_ptr<Packet>> get_to_be_sent_packet();
 
    /** Put a packet received by the network to the received packets */
    void put_received_packet(std::unique_ptr<Packet> packet);

    /** Put a new event */
    void put_nm_event(Event e);

    /** Adds a the comm activity to the node activities */
    void put_comm_activity(simgrid::s4u::CommPtr comm_activity);

    /** Check if the async get has an activity or not */
    bool is_empty_get();

    /** Test if the async get has finished and return the CommPtr if so */
    optional<simgrid::s4u::CommPtr> test_get();
};

#endif // !FALAFELS_MEDIATOR_PRODUCER_HPP
