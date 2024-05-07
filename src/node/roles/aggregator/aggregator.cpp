#include <xbt/asserts.h>
#include <xbt/log.h>

#include "aggregator.hpp"
#include "../../../constants.hpp"

XBT_LOG_NEW_DEFAULT_CATEGORY(s4u_aggregator, "Messages specific for this example");

/** 
 * Launch the aggregating activity or test if the current one has finished.
 * Returns true when aggregation is finished.
 */
bool Aggregator::aggregate() 
{
    // if aggregating activity doesn't exists
    if (this->aggregating_activity == nullptr)
    {
        /** Simulate the aggregation execution by multiplying the number of models to aggregate
            by a constant value indicating the number of flops for one model aggregation. */
        double flops = Constants::GLOBAL_MODEL_AGGREGATING_FLOPS * this->number_local_models;
        XBT_INFO("Starting aggregation with flops value: %f", flops);

        this->aggregating_activity = simgrid::s4u::this_actor::exec_async(flops);
        this->number_aggregated_models += this->number_local_models;
    }

    if (this->aggregating_activity->test())
    {
        this->aggregating_activity = nullptr;
        return true;
    }
    else
    {
        return false; 
    }
}

void Aggregator::print_end_report() 
{
    xbt_assert(this->number_client_training > 0, "Cannot print end report because the Aggregator had 0 or less client training.");

    this->number_global_epochs = this->number_aggregated_models / this->number_client_training;

    XBT_INFO("Number of model aggregated: %lu", this->number_aggregated_models);
    XBT_INFO("Number of client that were training: %u", this->number_client_training);
    XBT_INFO("Number of global epochs done: %u", this->number_global_epochs);
}
