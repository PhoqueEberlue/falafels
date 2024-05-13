#include "role.hpp"
#include <optional>

using namespace std;

XBT_LOG_NEW_DEFAULT_CATEGORY(s4u_role, "Messages specific for this example");

void Role::set_queues(
    shared_ptr<queue<unique_ptr<Packet>>> received, 
    shared_ptr<queue<shared_ptr<Packet>>> to_be_sent,
    shared_ptr<queue<unique_ptr<NetworkManager::Event>>> nm_events)
{
    this->received_packets = received;
    this->to_be_sent_packets = to_be_sent;
    this->nm_events = nm_events;
}

optional<unique_ptr<Packet>> Role::get_received_packet()
{
    if (this->received_packets->empty())
        return nullopt;

    auto p = std::move(this->received_packets->front());
    this->received_packets->pop();
    return p;
}

void Role::put_to_be_sent_packet(Packet packet)
{
    this->to_be_sent_packets->push(make_shared<Packet>(packet));
}

optional<unique_ptr<NetworkManager::Event>> Role::get_nm_event()
{
    if (this->nm_events->empty())
        return nullopt;

    auto p = std::move(this->nm_events->front());
    this->nm_events->pop();
    return p;
}
