/* Star Network Manager */
#ifndef FALAFELS_STAR_NM_HPP
#define FALAFELS_STAR_NM_HPP

#include "nm.hpp"
#include <memory>
#include <vector>

class StarNetworkManager : public NetworkManager 
{
private:
    using State = enum
    {
        INITIALIZING,
        WAITING_REGISTRATION_REQUEST,
        WAITING_REGISTRATION_CONFIRMATION,
        RUNNING,
        KILLING,
    };

    /** State of the NetworkManager */
    State state = INITIALIZING;

    std::vector<protocol::NodeInfo> *connected_nodes;
public:
    StarNetworkManager(protocol::NodeInfo);
    ~StarNetworkManager();
    
    // See nm.hpp for documentation
    void run();
    void handle_registration_requests();
    void send_registration_request();
    void handle_registration_confirmation(const protocol::operations::RegistrationConfirmation &confirmation);
    void broadcast(const std::unique_ptr<protocol::Packet> &packet, bool is_redirected=false);
    void handle_kill_phase();
};

#endif // !FALAFELS_STAR_NM_HPP
