#include <cstdint>
#include <memory>
#include <simgrid/s4u/Actor.hpp>
#include <simgrid/s4u/Engine.hpp>
#include <simgrid/s4u/Mailbox.hpp>
#include <variant>
#include <xbt/log.h>

#include "simple_aggregator.hpp"
#include "../../../protocol.hpp"
#include "aggregator.hpp"

using namespace std;

XBT_LOG_NEW_DEFAULT_CATEGORY(s4u_simple_aggregator, "Messages specific for this example");

SimpleAggregator::SimpleAggregator(std::unordered_map<std::string, std::string> *args)
{
    this->initialization_time = simgrid::s4u::Engine::get_instance()->get_clock();
    // No arguments yet
    this->args = args;
}

SimpleAggregator::~SimpleAggregator()
{
    delete args;
}

void SimpleAggregator::run()
{
    if (!this->still_has_activities)
        return;

    // Stop aggregating and send kills to the trainers
    if (simgrid::s4u::Engine::get_instance()->get_clock() > this->initialization_time + Constants::DURATION_TRAINING_PHASE)
    {
        this->send_kills();
        this->print_end_report();
        this->still_has_activities = false;
    }

    switch (this->state)
    {
        case INITIALIZING:
            if (auto e = this->get_nm_event())
            {
                if (auto *conneted_event = get_if<NetworkManager::ClusterConnected>(e->get()))
                {
                    this->number_client_training = conneted_event->number_client_connected;
                    this->send_global_model();
                    this->state = WAITING_LOCAL_MODELS;
                }
            }
            break;
        case WAITING_LOCAL_MODELS:
            if (auto packet = this->get_received_packet())
            {
                if (auto *send_local = get_if<Packet::SendLocalModel>(&(*packet)->op))
                {
                    this->number_local_models += 1;
                    XBT_INFO("nb local models: %lu", this->number_local_models);

                    if (this->number_local_models >= this->number_client_training)
                    {
                        this->state = AGGREGATING;
                    }
                }
            }
            break;
        case AGGREGATING:
            if (this->aggregate())
            {
                this->number_local_models = 0;
                this->send_global_model();
                this->state = WAITING_LOCAL_MODELS;
            }
            break;
    }
}


/* Sends the global model to every trainers */
void SimpleAggregator::send_global_model()
{
    this->put_to_be_sent_packet(
        Packet(
            // Send global model with broadcast because we specify a filter instead of a dst
            Filters::trainers_and_aggregators,
            Packet::SendGlobalModel(
                this->number_local_epochs
            )
        )
    );
}

void SimpleAggregator::send_kills()
{
    this->put_to_be_sent_packet(
        Packet(
            // Send kills with broadcast
            Filters::trainers_and_aggregators,
            Packet::KillTrainer()
        )
    );
}
