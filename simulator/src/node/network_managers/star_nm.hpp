/* Centralized Network Manager */
#ifndef FALAFELS_CENTRALIZED_NM_HPP
#define FALAFELS_CENTRALIZED_NM_HPP

#include "nm.hpp"
#include <cstdint>
#include <memory>
#include <vector>

class StarNetworkManager : public NetworkManager 
{
private:
    std::vector<NodeInfo> *connected_nodes;
public:
    StarNetworkManager(NodeInfo);
    ~StarNetworkManager();
    
    // See nm.hpp for documentation
    void handle_registration_requests();
    void send_registration_request();
    void handle_registration_confirmation(const Packet::RegistrationConfirmation &confirmation);
    void route_packet(Packet *packet);
    void broadcast(Packet *packet);
};

#endif // !FALAFELS_CENTRALIZED_NM_HPP
