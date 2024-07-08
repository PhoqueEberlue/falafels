#include "./s_mediator_consumer.hpp"
#include <xbt/log.h>


XBT_LOG_NEW_DEFAULT_CATEGORY(s4u_mediator_consumer, "Messages specific for this example");

using namespace std;
using namespace protocol;

unique_ptr<operations::Operation> SimulatedMediatorConsumer::get_received_operation()
{
    return this->mq_received_operations->get_unique<operations::Operation>();
}

void SimulatedMediatorConsumer::put_to_be_sent_packet(filters::NodeFilter filter, const operations::Operation op)
{
    auto p = new Packet(filter, op);
    this->mq_to_be_sent_packets->put(p);
}

void SimulatedMediatorConsumer::put_async_to_be_sent_packet(filters::NodeFilter filter, const operations::Operation op)
{
    auto p = new Packet(filter, op);
    auto mess = this->mq_to_be_sent_packets->put_async(p);
    this->async_messages->push(mess);
}

unique_ptr<events::NetworkEvent> SimulatedMediatorConsumer::get_nm_event()
{
    return this->mq_nm_events->get_unique<events::NetworkEvent>();
}

void SimulatedMediatorConsumer::wait_all_async_comms()
{
    this->async_messages->wait_all();
}
