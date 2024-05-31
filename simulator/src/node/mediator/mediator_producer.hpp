#ifndef FALAFELS_MEDIATOR_PRODUCER_HPP
#define FALAFELS_MEDIATOR_PRODUCER_HPP


#include "mediator.hpp"

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
    MediatorProducer(std::shared_ptr<std::queue<std::unique_ptr<Packet>>> received, 
                    std::shared_ptr<std::queue<std::shared_ptr<Packet>>> to_be_sent,
                    std::shared_ptr<std::queue<std::unique_ptr<Event>>> nm_events)
    : Mediator(received, to_be_sent, nm_events) {}

    /** Optionally get packet to be sent if some */
    std::optional<std::shared_ptr<Packet>> get_to_be_sent_packet()
    {
        if (this->to_be_sent_packets->empty())
            return nullopt;

        auto p = std::move(this->to_be_sent_packets->front());
        this->to_be_sent_packets->pop();
        return p;
    }

    /** Put a packet received by the network to the received packets */
    void put_received_packet(std::unique_ptr<Packet> packet)
    {
        this->received_packets->push(std::move(packet));
    }

    /** Put a new event */
    void put_nm_event(Event e)
    {
        this->nm_events->push(make_unique<Event>(e));
    }
};

#endif // !FALAFELS_MEDIATOR_PRODUCER_HPP
