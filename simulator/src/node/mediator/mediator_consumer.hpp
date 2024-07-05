#ifndef FALAFELS_MEDIATOR_CONSUMER_HPP
#define FALAFELS_MEDIATOR_CONSUMER_HPP

#include "mediator.hpp"

using namespace std;

/**
 * MediatorConsumer is the end to be used my Roles.
 * It can:
 * - Get packet received by the Networkmanager (The Producer)
 * - Get event from the Networkmanager
 * - Put packets to be sent by the Networkmanager
 */
class MediatorConsumer : public Mediator
{
public:
    MediatorConsumer(protocol::node_name name) : Mediator(name) {}

    /** Blocking get for retrieving a received operation */
    std::unique_ptr<protocol::operations::Operation> get_received_operation();

    /** Blocking put a packet to be sent by the NetWorkManager */
    void put_to_be_sent_packet(protocol::filters::NodeFilter filter, 
                               const protocol::operations::Operation op);

    /** Async put a packet to be sent by the NetWorkManager */
    void put_async_to_be_sent_packet(protocol::filters::NodeFilter filter, 
                                     const protocol::operations::Operation op);

    /** Blocking get for retrieving a NetworkManager Event */
    std::unique_ptr<Event> get_nm_event();
};

#endif // !FALAFELS_MEDIATOR_CONSUMER_HPP
