/* Network Manager */
#ifndef FALAFELS_NETWORK_MANAGER_HPP
#define FALAFELS_NETWORK_MANAGER_HPP

#include <functional>
#include <simgrid/s4u/Mailbox.hpp>
#include <vector>
#include "../../protocol.hpp"

class NetworkManager 
{
    protected:
        simgrid::s4u::Mailbox *mailbox;
        std::vector<NodeInfo*> *bootstrap_nodes;
        node_name my_node_name;

    public:
        NetworkManager(){}
        virtual ~NetworkManager(){}
        virtual void put(Packet*, node_name, uint64_t simulated_size_in_bytes) = 0;
        virtual bool put_timeout(Packet *packet, node_name name, uint64_t simulated_size_in_bytes, uint64_t timeout) = 0;

        virtual Packet *get() = 0;
        virtual std::vector<node_name> get_node_names_filter(std::function<bool(NodeInfo*)>) = 0;

        void set_bootstrap_nodes(std::vector<NodeInfo*> *nodes) { this->bootstrap_nodes = nodes; }
        std::vector<NodeInfo*> *get_bootstrap_nodes() { return this->bootstrap_nodes; }
        node_name get_my_node_name() { return this->my_node_name; }
};

#endif // !FALAFELS_NETWORK_MANAGER_HPP
