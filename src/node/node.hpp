#ifndef FALAFELS_NODE_HPP
#define FALAFELS_NODE_HPP

#include <memory>
#include <simgrid/s4u/Actor.hpp>
#include <simgrid/s4u/Engine.hpp>
#include <vector>
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
 */
class Node
{
private:
    std::unique_ptr<Role> role;
    std::vector<std::unique_ptr<NetworkManager>> *network_managers; 
public:
    /**
     * Node constructor
     * @param r Node's Role
     * @param nm Node's NetworkManager
     */
    Node(std::unique_ptr<Role> r, std::unique_ptr<NetworkManager> nm);

    /**
     * Node's NetworkManager and Role are deleted implicitly with the unique pointers.
     */
    ~Node() {};

    /**
     * Main function to execute the Node's behaviour.
     * Call the run function of the Role and NetworkManager.
     */
    void run();

    /**
     * Call set_bootstrap_nodes() of the Node's NetworkManager.
     */
    void set_bootstrap_nodes(std::vector<NodeInfo> *nodes);

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
     * Return a NodeInfo struct representing Node's information.
     * @return NodeInfo*.
     */
    NodeInfo get_node_info();
};

#endif // !FALAFELS_NODE_HPP
