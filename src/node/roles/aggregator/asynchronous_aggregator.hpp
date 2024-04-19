/* Aggregator */
#ifndef FALAFELS_ASYNCHRONOUS_AGGREGATOR_HPP
#define FALAFELS_ASYNCHRONOUS_AGGREGATOR_HPP

#include "aggregator.hpp"
#include <vector>

class AsynchronousAggregator : public Aggregator
{
private:
    /** Value between 0.0 and 1.0 indicating the proportion of local models needed before initializing an aggregation round */
    float proportion_threshold = 0.5;

    /** list of trainers node_name that finished their job (to the current node's knowledge) */
    std::vector<node_name> available_trainers;
    void send_global_model(node_name name);
    node_name wait_local_model();
    void send_kills();
    void broadcast_global_model();
public:
    AsynchronousAggregator(std::unordered_map<std::string, std::string> *args);
    void run();
};

#endif // !FALAFELS_ASYNCHRONOUS_AGGREGATOR_HPP
