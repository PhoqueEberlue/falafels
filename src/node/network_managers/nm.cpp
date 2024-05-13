#include "nm.hpp"
#include <algorithm>
#include <format>
#include <memory>
#include <optional>
#include <simgrid/Exception.hpp>
#include <simgrid/forward.h>
#include <simgrid/s4u/Activity.hpp>
#include <simgrid/s4u/Actor.hpp>
#include <simgrid/s4u/Engine.hpp>
#include <unordered_map>
#include <vector>
#include <xbt/log.h>
#include <xbt/asserts.h>
#include <xbt/ex.h>
#include <simgrid/s4u/ActivitySet.hpp>

#include "../../dot.hpp"

using namespace std;

XBT_LOG_NEW_DEFAULT_CATEGORY(s4u_network_manager, "Messages specific for this example");

NetworkManager::NetworkManager(NodeInfo node_info) : my_node_info(node_info)
{
    this->pending_async_put = new simgrid::s4u::ActivitySet();

    // Initializing our mailbox
    this->mailbox = simgrid::s4u::Mailbox::by_name(this->my_node_info.name);

    this->registration_requests = new vector<Packet::RegistrationRequest>();
}

NetworkManager::~NetworkManager()
{
    delete this->bootstrap_nodes;
    delete this->pending_async_put;
    delete this->registration_requests;
}

optional<shared_ptr<Packet>> NetworkManager::get_to_be_sent_packet()
{
    if (this->to_be_sent_packets->empty())
        return nullopt;

    auto p = std::move(this->to_be_sent_packets->front());
    this->to_be_sent_packets->pop();
    return p;
}

void NetworkManager::put_received_packet(unique_ptr<Packet> packet)
{
    this->received_packets->push(std::move(packet));
}

void NetworkManager::put_nm_event(Event e)
{
    this->nm_events->push(make_unique<Event>(e));
}

void NetworkManager::set_queues(
    shared_ptr<queue<unique_ptr<Packet>>> received, 
    shared_ptr<queue<shared_ptr<Packet>>> to_be_sent,
    shared_ptr<queue<unique_ptr<Event>>> nm_events)
{
    this->received_packets = received;
    this->to_be_sent_packets = to_be_sent;
    this->nm_events = nm_events;
}

void NetworkManager::set_bootstrap_nodes(vector<NodeInfo> *nodes)
{
    this->bootstrap_nodes = nodes;
}

bool NetworkManager::run()
{ 
    simgrid::s4u::this_actor::sleep_for(0.1);

    switch (this->state)
    {
        case INITIALIZING:
            switch (this->my_node_info.role)
            {
                case NodeRole::Trainer:
                    this->send_registration_request();
                    this->state = WAITING_REGISTRATION_CONFIRMATION;
                    break;
                case NodeRole::Aggregator:
                    this->state = WAITING_REGISTRATION_REQUEST;
                    break;
            }
            break;
        case WAITING_REGISTRATION_CONFIRMATION:
            if (auto packet = this->try_get())
            {
                if (auto *confirmation = get_if<Packet::RegistrationConfirmation>(&(*packet)->op))
                {
                    this->handle_registration_confirmation(*confirmation);
                    this->state = RUNNING;
                }
            }
            break;
        case WAITING_REGISTRATION_REQUEST:
            if (!this->start_time.has_value())
                this->start_time = simgrid::s4u::Engine::get_instance()->get_clock();

            if (auto packet = this->try_get())
            {
                if (auto *request = get_if<Packet::RegistrationRequest>(&(*packet)->op))
                {
                    this->registration_requests->push_back(*request);
                }

            }

            // At some point, stop listening for registration requests and end this state
            if (simgrid::s4u::Engine::get_instance()->get_clock() - *this->start_time > Constants::REGISTRATION_TIMEOUT) 
            {
                this->handle_registration_requests();
                this->state = RUNNING;
            }
            break;
        case RUNNING:
            if (auto packet = this->try_get())
            {
                // Case where we receive Kill
                if (auto *kill = get_if<Packet::KillTrainer>(&(*packet)->op))
                {
                    this->state = KILLING;
                }
                this->route_packet(std::move(*packet));
            }

            if (auto p = this->get_to_be_sent_packet())
            {
                // Case where we send kill to someone else
                if (auto *kill = get_if<Packet::KillTrainer>(&(*p)->op))
                {
                    this->state = KILLING;
                }

                this->send_packet(*p);
            }
            break;
        case KILLING:
            // TBH implementing kill isn't very interesting and will not play a lot in electric consumption.
            // It might be a loss of time.....
            this->pending_async_put->clear();
            //this->pending_async_put->wait_all_for(2);
            // this->wait_last_comms(1);
            // this->pending_async_put->wait_all();
            return false;
    }
    return true;
}

optional<unique_ptr<Packet>> NetworkManager::try_get()
{
    if (!this->pending_async_get)
        this->pending_async_get = this->mailbox->get_async();

    if (this->pending_async_get->test())
    {
        auto p = static_cast<Packet*>(this->pending_async_get->get_payload());
        XBT_INFO("%s <--%s(%lu)--- %s", p->dst.c_str(), p->get_op_name(), p->id, p->src.c_str());
        this->pending_async_get = this->mailbox->get_async();

        auto unique_packet = make_unique<Packet>(*p);
        delete p;
        return unique_packet;
    }
    else
    {
        return nullopt;
    }
}

void NetworkManager::send_packet(shared_ptr<Packet> p)
{
    if (p->broadcast)
    {
        this->broadcast(p);
    }
    else
    {
        this->send_async(p);
    }
}

void NetworkManager::send_async(shared_ptr<Packet> packet, bool is_redirected)
{
    auto p_clone = packet->clone();
    p_clone->src = this->get_my_node_name();
    p_clone->dst = packet->dst;

    // Only write original source when sending packets created by the current node.
    if (!is_redirected)
    {
        p_clone->original_src = this->get_my_node_name();
        XBT_INFO("%s ---%s(%lu)--> %s", p_clone->src.c_str(), p_clone->get_op_name(), p_clone->id, p_clone->dst.c_str());
    }
    else
    {
        p_clone->original_src = packet->original_src;
        XBT_INFO("%s ---%s(%lu)--> %s [REDIRECT]", p_clone->src.c_str(), p_clone->get_op_name(), p_clone->id, p_clone->dst.c_str());
    }

    auto receiver_mailbox = simgrid::s4u::Mailbox::by_name(p_clone->dst);

    DOTGenerator::get_instance().add_to_state(
        simgrid::s4u::Engine::get_instance()->get_clock(), 
        std::format("{} -> {} [label=\"{}\", style=dotted];", p_clone->src, p_clone->dst, p_clone->get_op_name())
    );


    auto comm = receiver_mailbox->put_async(p_clone, p_clone->get_packet_size());
    
    this->pending_async_put->push(comm);
}
