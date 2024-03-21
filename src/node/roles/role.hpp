/* Role */
#ifndef FALAFELS_ROLE_HPP
#define FALAFELS_ROLE_HPP

#include "../network_managers/nm.hpp"

/**
 * Abstract class that defines a Node behaviour in a Federated Learning system.
 * There are 3 different types of roles that inherits from Role class.
 * - Aggregator
 * - Trainer
 * - Proxy
 */
class Role
{
private:
    NetworkManager *network_manager;
public:
    Role(){}
    virtual ~Role(){}
    virtual void run() = 0;
    virtual NodeRole get_role_type() = 0;
    
    /**
     * Sets the NetworkManager of the current Role.
     * @param nm NetworkManager
     */
    void set_network_manager(NetworkManager *nm) { network_manager = nm; }

    /**
     * Get the NetworkManager of the current Role.
     * @return nm NetworkManager
     */
    NetworkManager *get_network_manager() { return network_manager; }
};

#endif // !FALAFELS_ROLE_HPP
