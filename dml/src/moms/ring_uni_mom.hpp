/* Unidirectional Ring Network Manager */
#ifndef FALAFELS_UNI_RING_NM_HPP
#define FALAFELS_UNI_RING_NM_HPP

#include "../../include/mom.hpp"
#include <memory>
#include <simgrid/forward.h>

class RingUniMOM : public MOM 
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

    /** Left neighbour of the current node */
    protocol::NodeInfo left_node;
    
    bool cluster_connected_have_been_sent = false;

    /** Redirect a packet to a neighbour of the ring */
    void redirect(const std::unique_ptr<protocol::Packet> &p);
public:
    RingUniMOM(protocol::NodeInfo);
    ~RingUniMOM();

    // See nm.hpp for documentation
    void run();
    void handle_registration_requests();
    void send_registration_request();
    void handle_registration_confirmation(const protocol::operations::RegistrationConfirmation &confirmation);
    void send_to_neighbour(const std::unique_ptr<protocol::Packet> &p, bool is_redirected=false);
    void broadcast(const std::unique_ptr<protocol::Packet> &packet, bool is_redirected=false) {}
    void handle_kill_phase();
};


#endif // !FALAFELS_UNI_RING_NM_HPP
