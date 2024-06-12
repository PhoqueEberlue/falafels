#ifndef FALAFELS_MEDIATOR_CONSUMER_HPP
#define FALAFELS_MEDIATOR_CONSUMER_HPP

#include "mediator.hpp"
#include <simgrid/forward.h>

using namespace std;

/**
 * MediatorConsumer is the end to be used by Roles.
 * It can:
 * - Get packet received by the Networkmanager (The Producer)
 * - Get event from the Networkmanager
 * - Put packets to be sent by the Networkmanager
 */
class MediatorConsumer : public Mediator
{
public:
    MediatorConsumer(std::shared_ptr<std::queue<std::unique_ptr<Packet>>> received, 
                     std::shared_ptr<std::queue<std::shared_ptr<Packet>>> to_be_sent,
                     std::shared_ptr<std::queue<std::unique_ptr<Event>>> nm_events,
                     simgrid::s4u::ActivitySet *node_activities)
    : Mediator(received, to_be_sent, nm_events, node_activities) {}

    /** Optionally get a Packet if available */
    std::optional<std::unique_ptr<Packet>> get_received_packet(); 

    /** Put a packet to be sent by the NetWorkManager */
    void put_to_be_sent_packet(Packet packet);

    /** Optionally get a NetworkManager Event if available */
    std::optional<std::unique_ptr<Event>> get_nm_event(); 

    /** Adds a the exec activity to the node activities */
    void put_exec_activity(simgrid::s4u::ExecPtr exec_activity);

    /** Test any of the activities that belongs to the Role */
    bool test_any_activies(); 

    /** Checks if there is any Role's activity */
    bool is_empty_activities(); 

    /** Clear only Role's activities */
    void clear_activities(); 
};

#endif // !FALAFELS_MEDIATOR_CONSUMER_HPP
