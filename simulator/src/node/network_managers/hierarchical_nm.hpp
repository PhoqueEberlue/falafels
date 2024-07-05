/* Hierarchical Network Manager */
#ifndef FALAFELS_HIERARCHICAL_NM_HPP
#define FALAFELS_HIERARCHICAL_NM_HPP

#include "nm.hpp"
#include <memory>
#include <vector>

class HierarchicalNetworkManager : public NetworkManager 
{
private:
    std::vector<protocol::NodeInfo> *connected_nodes;
public:
    HierarchicalNetworkManager(protocol::NodeInfo);
    ~HierarchicalNetworkManager();
    
    // See nm.hpp for documentation
    void run();
    void handle_registration_requests();
    void send_registration_request();
    void handle_registration_confirmation(const protocol::operations::RegistrationConfirmation &confirmation);
    void broadcast(const std::unique_ptr<protocol::Packet> &packet, bool is_redirected=false);
    void handle_kill_phase();
};

#endif // !FALAFELS_HIERARCHICAL_NM_HPP
