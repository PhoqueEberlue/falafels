#include "mediator_consumer.hpp"
#include <boost/smart_ptr/intrusive_ptr.hpp>
#include <optional>
#include <simgrid/forward.h>
#include <simgrid/s4u/Exec.hpp>

XBT_LOG_NEW_DEFAULT_CATEGORY(s4u_mediator_consumer, "Messages specific for this example");

std::optional<std::unique_ptr<Packet>> MediatorConsumer::get_received_packet()
{
    if (this->received_packets->empty())
        return nullopt;

    auto p = std::move(this->received_packets->front());
    this->received_packets->pop();
    return p;
}

void MediatorConsumer::put_to_be_sent_packet(Packet packet)
{
    this->to_be_sent_packets->push(make_shared<Packet>(packet));
}

std::optional<std::unique_ptr<Mediator::Event>> MediatorConsumer::get_nm_event()
{
    if (this->nm_events->empty())
        return nullopt;

    auto p = std::move(this->nm_events->front());
    this->nm_events->pop();
    return p;
}

void MediatorConsumer::put_exec_activity(simgrid::s4u::ExecPtr exec_activity)
{
    this->node_activities->push(
        boost::dynamic_pointer_cast<simgrid::s4u::Activity>(exec_activity)
    );
}

bool MediatorConsumer::test_any_activies()
{
    for (int i = 0; i<this->node_activities->size(); i++)
    {
        auto a = this->node_activities->at(i);

        // Check only for Exec activities
        if (boost::dynamic_pointer_cast<simgrid::s4u::Exec>(a))
        {
            if (a->is_done())
                return true;
        }
    }

    return false;
}

bool MediatorConsumer::is_empty_activities()
{
    for (int i = 0; i<this->node_activities->size(); i++)
    {
        auto a = this->node_activities->at(i);

        // If at least one Exec activity exists, its not empty
        if (boost::dynamic_pointer_cast<simgrid::s4u::Exec>(a))
        {
            return false;
        }
    }

    return true;
}

void MediatorConsumer::clear_activities()
{
    int i = 0;
    int end = this->node_activities->size() - 1;

    while (i <= end) {
        auto a = this->node_activities->at(i);

        // Check only for Exec activities
        if (boost::dynamic_pointer_cast<simgrid::s4u::Exec>(a))
        {
            this->node_activities->erase(a);
            end--;
        }
        else
        {
            i++;
        }
    }
}
