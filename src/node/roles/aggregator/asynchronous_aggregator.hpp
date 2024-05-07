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

    node_name src_save;
    node_name original_src_save;

    void send_global_model_to(node_name dst, node_name final_dst);
public:
    AsynchronousAggregator(std::unordered_map<std::string, std::string> *args);
    ~AsynchronousAggregator() {};
    void run();
};

#endif // !FALAFELS_ASYNCHRONOUS_AGGREGATOR_HPP
