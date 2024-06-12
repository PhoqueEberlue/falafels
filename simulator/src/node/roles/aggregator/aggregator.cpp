#include <simgrid/s4u/Engine.hpp>
#include <xbt/asserts.h>
#include <xbt/log.h>

#include "aggregator.hpp"
#include "../../../constants.hpp"

XBT_LOG_NEW_DEFAULT_CATEGORY(s4u_aggregator, "Messages specific for this example");

Aggregator::Aggregator(node_name name)
{
    this->initialization_time = simgrid::s4u::Engine::get_instance()->get_clock();
    this->my_node_name = name;
}

void Aggregator::launch_one_aggregation()
{
    XBT_INFO("Starting aggregation of model %lu", this->current_number_aggregated_models);
    this->current_number_aggregated_models += 1;

    double flops = Constants::GLOBAL_MODEL_AGGREGATING_FLOPS;

    int nb_core = simgrid::s4u::this_actor::get_host()->get_core_count();
    double nb_flops_per_aggregation = flops / nb_core;

    XBT_DEBUG("flops / nb_core = nb_flops_per_aggregation: %f / %i = %f", flops, nb_core, nb_flops_per_aggregation);
    
    // Launch exactly nb_core parallel tasks
    for (int i = 0; i < nb_core; i++)
    {
        this->mc->put_exec_activity(simgrid::s4u::this_actor::exec_async(nb_flops_per_aggregation));
    }
}

bool Aggregator::aggregate() 
{
    // if aggregating activity doesn't exists
    if (this->mc->is_empty_activities())
    {
        this->launch_one_aggregation();
    }

    // Test if any activity finished
    if (this->mc->test_any_activies())
    {
        // Because the workload is splitted evently between each cores, we know that when one activity finished,
        // every others have finished too.
        this->mc->clear_activities();

        // if we need to perform more model aggregation, start a new aggregation task
        if (this->current_number_aggregated_models < this->number_local_models)
        {
            this->launch_one_aggregation();
        }
        else
        {
            // Increment the number of aggregated models
            this->total_aggregated_models += this->number_local_models;
            // Reset the current number of aggregated models
            this->current_number_aggregated_models = 0;
            // Compute the number of global epochs
            this->number_global_epochs = this->total_aggregated_models / this->number_client_training;
            // Return true when the last activity have been finished
            return true;
        }
    }

    return false; 
}

void Aggregator::send_global_model()
{
    this->mc->put_to_be_sent_packet(
        Packet(
            // Send global model with broadcast because we specify a filter instead of a dst
            Filters::trainers_and_aggregators,
            Packet::SendGlobalModel(
                this->number_local_epochs
            )
        )
    );
}

void Aggregator::send_kills()
{
    this->mc->put_to_be_sent_packet(
        Packet(
            // Send kills with broadcast
            Filters::trainers_and_aggregators,
            Packet::KillTrainer()
        )
    );
}

bool Aggregator::check_end_condition()
{ 
    if (Constants::END_CONDITION_DURATION_TRAINING_PHASE != -1.0)
    {
        return simgrid::s4u::Engine::get_instance()->get_clock() > this->initialization_time + Constants::END_CONDITION_DURATION_TRAINING_PHASE;
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
    XBT_INFO("Number of model aggregated: %lu", this->total_aggregated_models);
    XBT_INFO("Number of client that were training: %u", this->number_client_training);
    XBT_INFO("Number of global epochs done: %u", this->number_global_epochs);
}

