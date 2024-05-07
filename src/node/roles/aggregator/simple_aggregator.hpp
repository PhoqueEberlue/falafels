/* Aggregator */
#ifndef FALAFELS_SIMPLE_AGGREGATOR_HPP
#define FALAFELS_SIMPLE_AGGREGATOR_HPP

#include "aggregator.hpp"

class SimpleAggregator : public Aggregator
{
public:
    SimpleAggregator(std::unordered_map<std::string, std::string> *args);
    ~SimpleAggregator() {}
    void run();
};

#endif // !FALAFELS_SIMPLE_AGGREGATOR_HPP
