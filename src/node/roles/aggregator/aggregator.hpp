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

    /** Value indicating the number of global epochs achieved by the aggregator */
    uint16_t number_global_epochs = 0;

    /** Number of local model aggregated, used to compute the global number of epochs. */
    uint64_t number_aggregated_model = 0;

    /** The actual number of trainers */
    uint16_t number_client_training = 0;

    void aggregate(uint64_t number_models);
    void print_end_report();
public:
    Aggregator() {}
    virtual ~Aggregator() {}
    virtual void run() = 0;
    NodeRole get_role_type() { return NodeRole::Aggregator; };
};

#endif // !FALAFELS_AGGREGATOR_HPP
