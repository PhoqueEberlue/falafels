#ifndef FALAFELS_MEDIATOR_CONSUMER_HPP
#define FALAFELS_MEDIATOR_CONSUMER_HPP

#include "mediator.hpp"

using namespace std;

class MediatorConsumer : public Mediator
{
public:
    MediatorConsumer(std::shared_ptr<std::queue<std::unique_ptr<Packet>>> received, 
                    std::shared_ptr<std::queue<std::shared_ptr<Packet>>> to_be_sent,
                    std::shared_ptr<std::queue<std::unique_ptr<Event>>> nm_events)
    : Mediator(received, to_be_sent, nm_events) {}

    /** Optionally get a Packet if available */
    std::optional<std::unique_ptr<Packet>> get_received_packet()
    {
        if (this->received_packets->empty())
            return nullopt;

        auto p = std::move(this->received_packets->front());
        this->received_packets->pop();
        return p;
    }

    /** Put a packet to be sent by the NetWorkManager */
    void put_to_be_sent_packet(Packet packet)
    {
        this->to_be_sent_packets->push(make_shared<Packet>(packet));
    }

    /** Optionally get a NMEvent if available */
    std::optional<std::unique_ptr<Event>> get_nm_event()
    {
        if (this->nm_events->empty())
            return nullopt;

        auto p = std::move(this->nm_events->front());
        this->nm_events->pop();
        return p;
    }
};

#endif // !FALAFELS_MEDIATOR_CONSUMER_HPP
