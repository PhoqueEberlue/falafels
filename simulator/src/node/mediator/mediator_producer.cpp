#include "mediator_producer.hpp"
#include <simgrid/s4u/Comm.hpp>
#include <xbt/log.h>

XBT_LOG_NEW_DEFAULT_CATEGORY(s4u_mediator_producer, "Messages specific for this example");

std::optional<std::shared_ptr<Packet>> MediatorProducer::get_to_be_sent_packet()
{
    if (this->to_be_sent_packets->empty())
        return nullopt;

    auto p = std::move(this->to_be_sent_packets->front());
    this->to_be_sent_packets->pop();
    return p;
}

void MediatorProducer::put_received_packet(std::unique_ptr<Packet> packet)
{
    this->received_packets->push(std::move(packet));
}

void MediatorProducer::put_nm_event(Event e)
{
    this->nm_events->push(make_unique<Event>(e));
}

void MediatorProducer::put_comm_activity(simgrid::s4u::CommPtr comm_activity)
{
    this->node_activities->push(
        boost::dynamic_pointer_cast<simgrid::s4u::Activity>(comm_activity)
    );
}

bool MediatorProducer::is_empty_get()
{
    for (int i = 0; i<this->node_activities->size(); i++)
    {
        auto a = this->node_activities->at(i);

        // Check only for Comm activities
        if (boost::dynamic_pointer_cast<simgrid::s4u::Comm>(a))
        {
            return false;
        }
    }

    return true;
}

optional<simgrid::s4u::CommPtr> MediatorProducer::test_get()
{
    for (int i = 0; i<this->node_activities->size(); i++)
    {
        auto a = this->node_activities->at(i);

        // Check only for Comm activities, there should be only one at the time.
        if (boost::dynamic_pointer_cast<simgrid::s4u::Comm>(a))
        {
            // If the activity has finished
            if (a->is_done())
            {
                // Erase the activity from the ActivitySet
                this->node_activities->erase(a);
                // Return by casting to correct type
                return boost::dynamic_pointer_cast<simgrid::s4u::Comm>(a);
            }

            return nullopt;
        }
    }

    xbt_assert(false, "Called test_get() but the Comm activity is not in the ActivitySet");
}
