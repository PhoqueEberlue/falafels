#include <simgrid/s4u/Engine.hpp>
#include <xbt/asserts.h>
#include <xbt/log.h>

#include "aggregator.hpp"
#include "../../../constants.hpp"

XBT_LOG_NEW_DEFAULT_CATEGORY(s4u_aggregator, "Messages specific for this example");

using namespace protocol;

Aggregator::Aggregator(node_name name)
{
    this->initialization_time = simgrid::s4u::Engine::get_instance()->get_clock();
    this->my_node_name = name;
    this->aggregating_activities = new simgrid::s4u::ActivitySet();
}

void Aggregator::aggregate() 
{
    double flops = Constants::GLOBAL_MODEL_AGGREGATING_FLOPS;

    int nb_core = simgrid::s4u::this_actor::get_host()->get_core_count();

    // Number for one aggregation splitted in one core
    double total_nb_flops_per_core = (flops / nb_core) * this->number_local_models;

    XBT_DEBUG("(flops / nb_core) * nb_local_models = total_nb_flops_per_core <-> (%f / %i) * %u = %f", 
              flops, nb_core, this->number_local_epochs, total_nb_flops_per_core);
    
    // Launch exactly nb_core parallel tasks
    for (int i = 0; i < nb_core; i++)
    {
        this->aggregating_activities->push(simgrid::s4u::this_actor::exec_async(total_nb_flops_per_core));
    }

    // Wait for the tasks to complete
    this->aggregating_activities->wait_all();
    // Increment the number of aggregated models
    this->total_aggregated_models += this->number_local_models;
    // Compute the number of global epochs
    this->number_global_epochs = this->total_aggregated_models / this->number_client_training;
}

void Aggregator::send_global_model()
{
    this->mc->put_async_to_be_sent_packet(
        // Send global model with broadcast because we specify a filter instead of a dst
        filters::trainers,
        operations::SendGlobalModel(
            this->number_local_epochs
        )
    );
}

void Aggregator::send_kills()
{
    this->mc->put_async_to_be_sent_packet(
        // Send kills with broadcast
        filters::everyone,
        operations::Kill()
    );

    this->mc->wait_all_async_comms();
}

bool Aggregator::check_end_condition()
{ 
    if (Constants::END_CONDITION_DURATION_TRAINING_PHASE != -1.0)
    {
        xbt_assert(false, "NOT IMPLEMENTED");
        // return simgrid::s4u::Engine::get_instance()->get_clock() > this->initialization_time + Constants::END_CONDITION_DURATION_TRAINING_PHASE;
    }
    else if (Constants::END_CONDITION_NUMBER_GLOBAL_EPOCHS != 0)
    {
        return this->number_global_epochs >= Constants::END_CONDITION_NUMBER_GLOBAL_EPOCHS;        
    }
    else
    {
        // Always crash when we reach this branch
        xbt_assert(false, "No END_CONDITION have been defined");
    }
}

void Aggregator::print_end_report() 
{
    XBT_INFO("---------------------------- End Report----------------------------------");
    XBT_INFO("Number of model aggregated: %lu", this->total_aggregated_models);
    XBT_INFO("Number of client that were training: %u", this->number_client_training);
    XBT_INFO("Number of global epochs done: %u", this->number_global_epochs);
    XBT_INFO("-------------------------------------------------------------------------");
}

