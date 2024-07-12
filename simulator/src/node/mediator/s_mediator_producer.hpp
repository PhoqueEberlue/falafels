#ifndef FALAFELS_S_MEDIATOR_PRODUCER_HPP
#define FALAFELS_S_MEDIATOR_PRODUCER_HPP


#include "s_mediator.hpp"
#include "../../../../dml/include/i_mediator_producer.hpp"
#include "../../../../dml/include/activity.hpp"

using namespace std;

/**
 * MediatorProducer is the end to be used my Networkmanagers.
 * It can:
 * - Put received packets
 * - Put Network events
 * - Get packets to be sent
 */
class SMediatorProducer : public IMediatorProducer, public SMediator
{
public:
    /** 
     * Initialize queues to enable communication between Role and NetworkManager.
     * The format for MessageQueue name is the following: `<node name>_mq_<message queue name>`.
     */
    SMediatorProducer(protocol::node_name name) : IMediatorProducer(), SMediator(name) {}

    /** -------------- IMPLEMENTATION OF VIRTUAL METHODS -------------- */
    /** Blocking get for retrieving a packet to be sent */
    std::shared_ptr<protocol::Packet> get_to_be_sent_packet();

    /** Async get for retrieving a packet to be sent */
    Activity get_async_to_be_sent_packet();

    /** Async put an operation received by the network */
    void put_received_operation(const protocol::operations::Operation op);

    /** Async put a new NetworkManager Event */
    void put_nm_event(protocol::events::NetworkEvent *e);

    void wait_all_async_comms();
    /** --------------------------------------------------------------- */
};

#endif // !FALAFELS_S_MEDIATOR_PRODUCER_HPP
