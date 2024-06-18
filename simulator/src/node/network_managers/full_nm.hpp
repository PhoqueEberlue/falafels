/* Fully-connected Network Manager */
#ifndef FALAFELS_FULLYCONNECTED_NM_HPP
#define FALAFELS_FULLYCONNECTED_NM_HPP

#include "nm.hpp"
#include <cstdint>
#include <memory>
#include <vector>

class FullyConnectedNetworkManager : public NetworkManager 
{
private:
    std::vector<NodeInfo> *connected_nodes;
public:
    FullyConnectedNetworkManager(NodeInfo);
    ~FullyConnectedNetworkManager();
    
    // See nm.hpp for documentation
    void handle_registration_requests();
    void send_registration_request();
    void handle_registration_confirmation(const Packet::RegistrationConfirmation &confirmation);
    void route_packet(Packet *p);
    void broadcast(Packet *p);
};

#endif // !FALAFELS_FULLYCONNECTED_NM_HPP
