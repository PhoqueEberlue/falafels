/* Centralized Network Manager */
#ifndef FALAFELS_CENTRALIZED_NM_HPP
#define FALAFELS_CENTRALIZED_NM_HPP

#include "nm.hpp"

class CentralizedNetworkManager : public NetworkManager 
{
private:
    ~CentralizedNetworkManager();
    void put(Packet*, node_name);
    bool put_timeout(Packet *packet, node_name name, uint64_t timeout);
    Packet *get();


    std::vector<node_name> get_node_names_filter(std::function<bool(NodeInfo*)>);
public:
    CentralizedNetworkManager(node_name);
    void run();
};

#endif // !FALAFELS_CENTRALIZED_NM_HPP
