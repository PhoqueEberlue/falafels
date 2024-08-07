#ifndef FALAFELS_MEDIATOR_PRODUCER_HPP
#define FALAFELS_MEDIATOR_PRODUCER_HPP


#include "mediator.hpp"
#include <simgrid/s4u/Mess.hpp>

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
    MediatorProducer(protocol::node_name name) : Mediator(name) {}

    /** Blocking get for retrieving a packet to be sent */
    std::shared_ptr<protocol::Packet> get_to_be_sent_packet();

    /** Async get for retrieving a packet to be sent */
    simgrid::s4u::MessPtr get_async_to_be_sent_packet();

    /** Async put an operation received by the network */
    void put_received_operation(const protocol::operations::Operation op);

    /** Async put a new NetworkManager Event */
    void put_nm_event(Event *e);
};

#endif // !FALAFELS_MEDIATOR_PRODUCER_HPP
