/* Aggregator */
#ifndef FALAFELS_ASYNCHRONOUS_AGGREGATOR_HPP
#define FALAFELS_ASYNCHRONOUS_AGGREGATOR_HPP

#include "aggregator.hpp"
#include <vector>

class AsynchronousAggregator : public Aggregator
{
private:
    uint32_t total_number_clients;

    /** Value between 0.0 and 1.0 indicating the proportion of local models needed before initializing an aggregation round */
    float proportion_threshold = 0.5;

    /** list of trainers node_name that finished their job (to the current node's knowledge) */
    std::vector<node_name> available_trainers;
    void send_global_model_to_available_trainers();
    uint64_t wait_local_models();
    void send_kills();
public:
    AsynchronousAggregator(float proportion_threshold);
    void run();
};

#endif // !FALAFELS_ASYNCHRONOUS_AGGREGATOR_HPP
