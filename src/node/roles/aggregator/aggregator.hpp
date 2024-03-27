/* Aggregator */
#ifndef FALAFELS_AGGREGATOR_HPP
#define FALAFELS_AGGREGATOR_HPP

#include "../role.hpp"
#include <cstdint>

class Aggregator : public Role 
{
protected:
    /** Value indicating the number of local epochs that the aggregator will ask the trainers to do. */
    uint8_t number_local_epochs = 3;

    void aggregate(uint64_t number_models);
    // TODO: what functions are in common with every aggregator??
    // virtual void send_global_model();
    // virtual void wait_local_models();
    // virtual void send_kills();
public:
    Aggregator() {}
    virtual ~Aggregator() {}
    virtual void run() = 0;
    NodeRole get_role_type() { return NodeRole::Aggregator; };
};

#endif // !FALAFELS_AGGREGATOR_HPP
