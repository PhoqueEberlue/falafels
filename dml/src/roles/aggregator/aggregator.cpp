#include "aggregator.hpp"
#include "../../../include/constants.hpp"
#include <cstdlib>
#include <iostream>

XBT_LOG_NEW_DEFAULT_CATEGORY(s4u_aggregator, "Messages specific for this example");

using namespace protocol;

Aggregator::Aggregator(node_name name)
{
    this->initialization_time = this->commons->get_time();
    this->my_node_name = name;
}

void Aggregator::aggregate() 
{
    double flops = Constants::GLOBAL_MODEL_AGGREGATING_FLOPS;

    int nb_core = this->executor->get_core_count();

    // Number for one aggregation splitted in one core
    double total_nb_flops_per_core = (flops / nb_core) * this->number_local_models;

    XBT_DEBUG("(flops / nb_core) * nb_local_models = total_nb_flops_per_core <-> (%f / %i) * %u = %f", 
              flops, nb_core, this->number_local_epochs, total_nb_flops_per_core);
    
    // Launch exactly nb_core parallel tasks
    for (int i = 0; i < nb_core; i++)
    {
        this->executor->exec_async(total_nb_flops_per_core);
    }

    // Wait for the executions to complete
    this->executor->wait_all_executions();
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
        protocol::operations::SendGlobalModel { 
            .number_local_epochs=this->number_local_epochs 
        }
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
    if (Constants::END_CONDITION_DURATION_TRAINING_PHASE != 0.0)
    {
        std::cout << "FIX XBT_DIE PLS" << std::endl; // TODO
        exit(1);
        // xbt_die("NOT IMPLEMENTED");
        // return simgrid::s4u::Engine::get_instance()->get_clock() > this->initialization_time + Constants::END_CONDITION_DURATION_TRAINING_PHASE;
    }
    else if (Constants::END_CONDITION_NUMBER_ROUNDS != 0)
    {
        return this->number_global_epochs >= Constants::END_CONDITION_NUMBER_ROUNDS;
    }
    else if (Constants::END_CONDITION_TOTAL_NUMBER_LOCAL_EPOCHS != 0)
    {
        return this->total_number_local_epochs >= Constants::END_CONDITION_TOTAL_NUMBER_LOCAL_EPOCHS;
    }
    else
    {
        // Always crash when we reach this branch
        std::cout << "FIX XBT_DIE PLS" << std::endl; // TODO
        exit(1);
        // xbt_die("No END_CONDITION have been defined");
    }
}

void Aggregator::print_end_report()
{
    XBT_INFO("---------------------------- End Report----------------------------------");
    XBT_INFO("Total number of local epochs: %lu", this->total_number_local_epochs);
    XBT_INFO("Number of model aggregated: %lu", this->total_aggregated_models);
    XBT_INFO("Number of client that were training: %u", this->number_client_training);
    XBT_INFO("Number of global epochs done: %u", this->number_global_epochs);
    XBT_INFO("-------------------------------------------------------------------------");
}

