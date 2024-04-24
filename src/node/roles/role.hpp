/* Role */
#ifndef FALAFELS_ROLE_HPP
#define FALAFELS_ROLE_HPP

#include "../network_managers/nm.hpp"
#include <memory>

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
    std::unique_ptr<NetworkManager> network_manager;
public:
    Role(){}
    virtual ~Role(){}
    virtual void run() = 0;
    virtual NodeRole get_role_type() = 0;
    
    /**
     * Sets the NetworkManager of the current Role.
     * @param nm NetworkManager
     */
    void set_network_manager(std::unique_ptr<NetworkManager> nm) { this->network_manager = std::move(nm); }

    /**
     * Get the NetworkManager of the current Role as a raw pointer (non-owning).
     * @return nm NetworkManager
     */
    NetworkManager *get_network_manager() { return this->network_manager.get(); }
};

#endif // !FALAFELS_ROLE_HPP
