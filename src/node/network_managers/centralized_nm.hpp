/* Centralized Network Manager */
#ifndef FALAFELS_CENTRALIZED_NM_HPP
#define FALAFELS_CENTRALIZED_NM_HPP

#include "nm.hpp"

class CentralizedNetworkManager : public NetworkManager 
{
    private:
        ~CentralizedNetworkManager();
        void send(Packet*, node_name);
        Packet *get();
        std::vector<node_name> get_node_names_filter(std::function<bool(NodeInfo*)>);
    public:
        CentralizedNetworkManager(std::vector<NodeInfo*>*, node_name);
        void run();
};

#endif // !FALAFELS_CENTRALIZED_NM_HPP
