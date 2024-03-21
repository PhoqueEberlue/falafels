/* Aggregator */
#ifndef FALAFELS_SIMPLE_AGGREGATOR_HPP
#define FALAFELS_SIMPLE_AGGREGATOR_HPP

#include "aggregator.hpp"

class SimpleAggregator : public Aggregator
{
    private:
        uint64_t number_client_training;
        void aggregate();
        void send_global_model();
        void wait_local_models();
        void send_kills();
    public:
        SimpleAggregator();
        void run();
};

#endif // !FALAFELS_SIMPLE_AGGREGATOR_HPP
