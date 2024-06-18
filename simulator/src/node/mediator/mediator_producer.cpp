#include "mediator_producer.hpp"

using namespace std;

XBT_LOG_NEW_DEFAULT_CATEGORY(s4u_mediator_producer, "Messages specific for this example");

shared_ptr<Packet> MediatorProducer::get_to_be_sent_packet()
{
    auto tmp = this->mq_to_be_sent_packets->get<Packet>();
    auto res = std::make_shared<Packet>(*tmp);
    delete tmp;

    return res;
}

/** Async get for retrieving a packet to be sent */
simgrid::s4u::CommPtr MediatorProducer::get_async_to_be_sent_packet()
{
    return this->mq_to_be_sent_packets->get_async();
}


/** Async put a packet received by the network */
void MediatorProducer::put_received_packet(Packet *packet)
{
    auto mess = this->mq_received_packets->put_async(packet, 0);
    this->async_messages->push(mess);
}

/** Async put a new NetworkManager Event */
void MediatorProducer::put_nm_event(Event *e)
{
    auto mess = this->mq_nm_events->put_async(e, 0);
    this->async_messages->push(mess);
}
