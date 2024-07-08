#ifndef DML_I_MEDIATOR_CONSUMER_HPP
#define DML_I_MEDIATOR_CONSUMER_HPP

#include "./protocol.hpp"

/**
 * IMediatorConsumer is the end to be used my Roles.
 * It can:
 * - Get packet received by the Networkmanager (The Producer)
 * - Get event from the Networkmanager
 * - Put packets to be sent by the Networkmanager
 */
class IMediatorConsumer
{
public:
    IMediatorConsumer() {}
    virtual ~IMediatorConsumer() {}

    /** Blocking get for retrieving a received operation */
    virtual std::unique_ptr<protocol::operations::Operation> get_received_operation() = 0;

    /** Blocking put a packet to be sent by the NetWorkManager */
    virtual void put_to_be_sent_packet(protocol::filters::NodeFilter filter, 
                               const protocol::operations::Operation op) = 0;

    /** Async put a packet to be sent by the NetWorkManager */
    virtual void put_async_to_be_sent_packet(protocol::filters::NodeFilter filter, 
                                     const protocol::operations::Operation op) = 0;

    /** Blocking get for retrieving a NetworkManager Event */
    virtual std::unique_ptr<protocol::events::NetworkEvent> get_nm_event() = 0;

    virtual void wait_all_async_comms() = 0;
};

#endif // !DML_I_MEDIATOR_CONSUMER_HPP
