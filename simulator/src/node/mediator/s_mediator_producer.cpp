#include "./s_mediator_producer.hpp"

XBT_LOG_NEW_DEFAULT_CATEGORY(s4u_mediator_producer, "Messages specific for this example");

using namespace std;
using namespace protocol;

shared_ptr<Packet> SMediatorProducer::get_to_be_sent_packet()
{
    auto tmp = this->mq_to_be_sent_packets->get<Packet>();
    auto res = std::make_shared<Packet>(*tmp);
    delete tmp;

    return res;
}

AsyncWrapper SMediatorProducer::get_async_to_be_sent_packet()
{
    // return this->mq_to_be_sent_packets->get_async();
}


void SMediatorProducer::put_received_operation(const operations::Operation op)
{
    auto mess = this->mq_received_operations->put_async(
        new operations::Operation(op) // Create heap allocated object
    );

    this->async_messages->push(mess);
}

void SMediatorProducer::put_nm_event(protocol::events::NetworkEvent *e)
{
    auto mess = this->mq_nm_events->put_async(e);
    this->async_messages->push(mess);
}

void SMediatorProducer::wait_all_async_comms()
{
    this->async_messages->wait_all();
}
