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

    void send_global_model(node_name dst, node_name final_dst);
    std::tuple<node_name, node_name> wait_local_model();
    void send_kills();
    void broadcast_global_model();
public:
    AsynchronousAggregator(std::unordered_map<std::string, std::string> *args);
    ~AsynchronousAggregator() {};
    void run();
};

#endif // !FALAFELS_ASYNCHRONOUS_AGGREGATOR_HPP
