/* Aggregator */
#ifndef FALAFELS_SIMPLE_AGGREGATOR_HPP
#define FALAFELS_SIMPLE_AGGREGATOR_HPP

#include "aggregator.hpp"

class SimpleAggregator : public Aggregator
{
private:
    // Arguments passed to the constructor.
    // They are kept here because subclass could have to use them.
    // Its the highest class responsability to delete them.
    std::unordered_map<std::string, std::string> *args;
protected:
    simgrid::s4u::ExecPtr training_activity;
    void send_global_model();
    uint64_t wait_local_models();
    void send_kills();
public:
    SimpleAggregator(std::unordered_map<std::string, std::string> *args);
    ~SimpleAggregator();
    bool run();
};

#endif // !FALAFELS_SIMPLE_AGGREGATOR_HPP
