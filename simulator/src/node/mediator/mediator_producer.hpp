#ifndef FALAFELS_MEDIATOR_PRODUCER_HPP
#define FALAFELS_MEDIATOR_PRODUCER_HPP


#include "mediator.hpp"
#include <simgrid/forward.h>
// #include <simgrid/s4u/Mess.hpp>

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
    MediatorProducer(node_name name) : Mediator(name) {}

    /** Blocking get for retrieving a packet to be sent */
    std::shared_ptr<Packet> get_to_be_sent_packet();

    /** Async get for retrieving a packet to be sent */
    simgrid::s4u::CommPtr get_async_to_be_sent_packet();

    /** Async put a packet received by the network */
    void put_received_packet(Packet *packet);

    /** Async put a new NetworkManager Event */
    void put_nm_event(Event *e);
};

#endif // !FALAFELS_MEDIATOR_PRODUCER_HPP
