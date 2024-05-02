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
public:
    Role(){}
    virtual ~Role(){}
    virtual bool run() = 0;
    virtual NodeRole get_role_type() = 0;

    std::optional<std::unique_ptr<Packet>> get_received_packet();
    void put_to_be_sent_packet(std::shared_ptr<Packet> packet);
    void set_queues(std::shared_ptr<std::queue<std::unique_ptr<Packet>>> received, 
                    std::shared_ptr<std::queue<std::shared_ptr<Packet>>> to_be_sent);
};

#endif // !FALAFELS_ROLE_HPP
