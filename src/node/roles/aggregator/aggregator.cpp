#include <xbt/log.h>

#include "aggregator.hpp"
#include "../../../constants.hpp"

XBT_LOG_NEW_DEFAULT_CATEGORY(s4u_aggregator, "Messages specific for this example");

void Aggregator::aggregate()
{
    double flops = constants::aggregator::AGGREGATION_FLOPS;
    XBT_INFO("Starting aggregation with flops value: %f", flops);
    simgrid::s4u::this_actor::execute(flops);
}
