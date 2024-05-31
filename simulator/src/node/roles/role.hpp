/* Role */
#ifndef FALAFELS_ROLE_HPP
#define FALAFELS_ROLE_HPP

#include "../mediator/mediator_consumer.hpp"
#include <memory>

/**
 * Abstract class that defines a Node behaviour in a Federated Learning system.
 * There are 3 different types of roles that inherits from Role class.
 * - Aggregator: Aggregates the local models into a global one.
 * - Trainer: Trains a local model.
 * - Proxy: Redirect communications between clusters (HDFL/DFL).
 */
class Role
{
protected:
    unique_ptr<MediatorConsumer> mc;

    node_name my_node_name;
public:
    Role(){}
    virtual ~Role(){} 

    void set_mediator_consumer(std::unique_ptr<MediatorConsumer> mc) { this->mc = std::move(mc); }

    /* --- Functions to be implemented by the children classes --- */
    virtual void run() = 0;
    virtual NodeRole get_role_type() = 0;
    /* ----------------------------------------------------------- */
};

#endif // !FALAFELS_ROLE_HPP
