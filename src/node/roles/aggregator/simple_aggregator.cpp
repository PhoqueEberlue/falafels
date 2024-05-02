#include <cstdint>
#include <memory>
#include <simgrid/s4u/Engine.hpp>
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

bool SimpleAggregator::run()
{
    // SET THIS
    this->number_client_training = NULL; // TODO

    // Stop aggregating and send kills to the trainers
    if (simgrid::s4u::Engine::get_instance()->get_clock() > this->initialization_time + Constants::DURATION_TRAINING_PHASE)
    {
        this->send_kills();
        this->print_end_report();
        return false;
    }

    switch (this->state)
    {
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

    return true;
}


/* Sends the global model to every trainers */
void SimpleAggregator::send_global_model()
{
    // Send global model with broadcast
    auto p = make_shared<Packet>(Packet(
        Filters::trainers_and_aggregators,
        Packet::SendGlobalModel(
            this->number_local_epochs
        )
    ));

    this->put_to_be_sent_packet(p);
}

void SimpleAggregator::send_kills()
{
    // Send kills with broadcast
    auto p = make_shared<Packet>(Packet(
        Filters::trainers_and_aggregators,
        Packet::KillTrainer()
    ));

    this->put_to_be_sent_packet(p);
}
