/* Aggregator */
#ifndef FALAFELS_AGGREGATOR_HPP
#define FALAFELS_AGGREGATOR_HPP

#include "role.hpp"

class Aggregator : public Role 
{
    private:
        void aggregate();
        void send_global_model();
        void wait_local_models();
        void send_kills();
    public:
        Aggregator();
        void run();
        NodeRole get_role_name() { return NodeRole::Aggregator; };
};

#endif // !FALAFELS_AGGREGATOR_HPP
