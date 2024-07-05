/* Aggregator */
#ifndef FALAFELS_SIMPLE_AGGREGATOR_HPP
#define FALAFELS_SIMPLE_AGGREGATOR_HPP

#include "aggregator.hpp"

class SimpleAggregator : public Aggregator
{
private:
    using State = enum
    {
        INITIALIZING,
        WAITING_LOCAL_MODELS,
        AGGREGATING,
    };

    /** State of the Aggregator */
    State state = INITIALIZING;

    bool first_global_model = true;
public:
    SimpleAggregator(std::unordered_map<std::string, std::string> *args, protocol::node_name name);
    ~SimpleAggregator() {}
    void run();
};

#endif // !FALAFELS_SIMPLE_AGGREGATOR_HPP
