/* Aggregator */
#ifndef FALAFELS_AGGREGATOR_HPP
#define FALAFELS_AGGREGATOR_HPP

#include "../role.hpp"

class Aggregator : public Role 
{
    private:
        // what functions are essential??
        // virtual void aggregate() = 0;
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
