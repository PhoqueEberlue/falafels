/* Aggregator */
#ifndef FALAFELS_SIMPLE_AGGREGATOR_HPP
#define FALAFELS_SIMPLE_AGGREGATOR_HPP

#include "aggregator.hpp"

class SimpleAggregator : public Aggregator
{
private:
    uint64_t number_client_training = 0;
    void send_global_model();
    uint64_t wait_local_models();
    void send_kills();
public:
    SimpleAggregator(std::unordered_map<std::string, std::string> *args);
    void run();
};

#endif // !FALAFELS_SIMPLE_AGGREGATOR_HPP
