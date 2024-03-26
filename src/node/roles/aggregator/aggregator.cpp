#include <xbt/log.h>

#include "aggregator.hpp"
#include "../../../constants.hpp"

XBT_LOG_NEW_DEFAULT_CATEGORY(s4u_aggregator, "Messages specific for this example");

/**
 * Simulate the aggregation execution by multiplying the number of models to aggregate
 * by a constant value indicating the number of flops for one model aggregation.
 */
void Aggregator::aggregate(uint64_t number_models)
{
    double flops = constants::aggregator::AGGREGATION_FLOPS * number_models;
    XBT_INFO("Starting aggregation with flops value: %f", flops);
    simgrid::s4u::this_actor::execute(flops);
}
