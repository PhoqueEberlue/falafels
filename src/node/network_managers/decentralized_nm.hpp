/* Decentralized Network Manager */
#ifndef FALAFELS_DECENTRALIZED_NM_HPP
#define FALAFELS_DECENTRALIZED_NM_HPP

#include "nm.hpp"

class DecentralizedNetworkManager : public NetworkManager
{
    private:
        ~DecentralizedNetworkManager();
        void send(Packet*, node_name);
        Packet *get();
        std::vector<node_name> get_node_names_filter(std::function<NodeInfo*(bool)>);
    public:
        DecentralizedNetworkManager(std::vector<NodeInfo*>*, node_name);
        void run();
};

#endif // !FALAFELS_DECENTRALIZED_NM_HPP
