/* Decentralized Network Manager */
#ifndef FALAFELS_DECENTRALIZED_NM_HPP
#define FALAFELS_DECENTRALIZED_NM_HPP

#include "nm.hpp"

class DecentralizedNetworkManager : public NetworkManager
{
    private:
        ~DecentralizedNetworkManager();
        void put(Packet*, node_name);
        bool put_timeout(Packet *packet, node_name name, uint64_t timeout);
        Packet *get();
        std::vector<node_name> get_node_names_filter(std::function<NodeInfo*(bool)>);
    public:
        DecentralizedNetworkManager(node_name);
        void run();
};

#endif // !FALAFELS_DECENTRALIZED_NM_HPP
