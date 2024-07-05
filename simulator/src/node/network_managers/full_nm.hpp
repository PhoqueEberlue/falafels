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
    std::vector<protocol::NodeInfo> *connected_nodes;
public:
    FullyConnectedNetworkManager(protocol::NodeInfo);
    ~FullyConnectedNetworkManager();
    
    // See nm.hpp for documentation
    void run();
    void handle_registration_requests();
    void send_registration_request();
    void handle_registration_confirmation(const protocol::operations::RegistrationConfirmation &confirmation);
    void route_packet(std::unique_ptr<protocol::Packet> p);
    void broadcast(const std::unique_ptr<protocol::Packet> &p, bool is_redirected=false);
    void handle_kill_phase();
};

#endif // !FALAFELS_FULLYCONNECTED_NM_HPP
