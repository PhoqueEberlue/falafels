#ifndef DML_I_MEDIATOR_PRODUCER_HPP
#define DML_I_MEDIATOR_PRODUCER_HPP

#include "./protocol.hpp"
#include "./activity.hpp"

/**
 * IMediatorProducer is the end to be used my MOMs.
 * It can:
 * - Put received packets
 * - Put Network events
 * - Get packets to be sent
 */
class IMediatorProducer
{
public:
    IMediatorProducer() {} 
    virtual ~IMediatorProducer() {}

    /** Blocking get for retrieving a packet to be sent */
    virtual std::shared_ptr<protocol::Packet> get_to_be_sent_packet() = 0;

    /** Async get for retrieving a packet to be sent */
    virtual Activity get_async_to_be_sent_packet() = 0;

    /** Async put an operation received by the network */
    virtual void put_received_operation(const protocol::operations::Operation op) = 0;

    /** Async put a new NetworkManager Event */
    virtual void put_nm_event(protocol::events::NetworkEvent *e) = 0;

    virtual void wait_all_async_comms() = 0;
};

#endif // !DML_I_MEDIATOR_PRODUCER_HPP
