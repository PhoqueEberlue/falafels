/* Role */
#ifndef FALAFELS_ROLE_HPP
#define FALAFELS_ROLE_HPP

#include "../network_managers/nm.hpp"
#include <memory>
#include <optional>
#include <queue>

/**
 * Abstract class that defines a Node behaviour in a Federated Learning system.
 * There are 3 different types of roles that inherits from Role class.
 * - Aggregator
 * - Trainer
 * - Proxy
 */
class Role
{
private:
    std::shared_ptr<std::queue<std::unique_ptr<Packet>>> received_packets;
    std::shared_ptr<std::queue<std::shared_ptr<Packet>>> to_be_sent_packets;
    std::shared_ptr<std::queue<std::unique_ptr<NetworkManager::Event>>> nm_events;
public:
    bool still_has_activities = true;
    Role(){}
    virtual ~Role(){}
    virtual void run() = 0;
    virtual NodeRole get_role_type() = 0;

    std::optional<std::unique_ptr<Packet>> get_received_packet();
    void put_to_be_sent_packet(Packet packet);
    std::optional<std::unique_ptr<NetworkManager::Event>> get_nm_event();
    void set_queues(std::shared_ptr<std::queue<std::unique_ptr<Packet>>> received, 
                    std::shared_ptr<std::queue<std::shared_ptr<Packet>>> to_be_sent,
                    std::shared_ptr<std::queue<std::unique_ptr<NetworkManager::Event>>> nm_events);
};

#endif // !FALAFELS_ROLE_HPP
