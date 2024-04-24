#ifndef FALAFELS_NODE_HPP
#define FALAFELS_NODE_HPP

#include <memory>
#include <simgrid/s4u/Actor.hpp>
#include <simgrid/s4u/Engine.hpp>
#include <xbt/log.h>
#include "roles/role.hpp"

/**
 * A Node represent an entity in the simulation world of falafels.
 * It is a highly flexible object designed to support various behaviours for a Federated Learning node.
 * The constraints while designing this architecture were:
 * - A node can have different roles: trainer, aggregator or proxy
 * - In a decentralized scenario a node can change its role at runtime
 * - Each role should be extandable in the future, i.e. we want multiple aggregators implementations (Sync, Async, Edge...)
 * - We want to support multiple networking algoritms in order to work with centralized, decentralized and semi-decentralized Federated Learning.
 * 
 * Thus, a node is constituted of a Role and a NetworkManager.
 * The NetworkManager is actually stored in the Role because this last need it... though we might want to change that later by sharing the NetworkManager's reference.
 */
class Node
{
private:
    std::unique_ptr<Role> role;
public:
    /**
     * Node constructor
     * @param r Node's Role
     * @param nm Node's NetworkManager
     */
    Node(std::unique_ptr<Role> r, std::unique_ptr<NetworkManager> nm);

    Node() {};
    /**
     * Delete node's NetworkManager and Role.
     */
    ~Node();

    /**
     * Main function to execute the Node's behaviour.
     * Call the run function of the associated role of the node.
     */
    void run() { this->role->run(); }

    /**
     * Set the Node's Role.
     * @param r New Role for the current Node.
     */
    void set_role(Role *r);

    /**
     * Get the Node's Role.
     * @return Node's Role.
     */
    Role *get_role() { return role.get(); }

    /**
     * Allocate a NodeInfo struct representing Node's information.
     * @return NodeInfo*.
     */
    NodeInfo get_node_info();
};

#endif // !FALAFELS_NODE_HPP
