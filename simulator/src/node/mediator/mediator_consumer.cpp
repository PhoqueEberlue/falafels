#include "mediator_consumer.hpp"
#include <xbt/log.h>

using namespace std;

XBT_LOG_NEW_DEFAULT_CATEGORY(s4u_mediator_consumer, "Messages specific for this example");

unique_ptr<Packet> MediatorConsumer::get_received_packet()
{
    return this->mq_received_packets->get_unique<Packet>();
}

/** Async put a packet to be sent by the NetWorkManager */
void MediatorConsumer::put_to_be_sent_packet(Packet *packet)
{
    auto mess = this->mq_to_be_sent_packets->put_async(packet, 0);
    this->async_messages->push(mess);
}

/** Blocking get for retrieving a NetworkManager Event */
unique_ptr<Mediator::Event> MediatorConsumer::get_nm_event()
{
    return this->mq_nm_events->get_unique<Event>();
}
