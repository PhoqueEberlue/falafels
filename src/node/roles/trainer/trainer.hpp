/* Trainer */
#ifndef FALAFELS_TRAINER_HPP
#define FALAFELS_TRAINER_HPP

#include "../role.hpp"
#include <cstdint>
#include <unordered_map>

class Trainer : public Role 
{
private:
    void train(uint8_t number_local_epochs);
    void send_local_model(node_name dst, node_name final_dst);
public:
    Trainer(std::unordered_map<std::string, std::string> *args);
    ~Trainer() {};
    void run();
    NodeRole get_role_type() { return NodeRole::Trainer; };
};

#endif //! FALAFELS_TRAINER_HPP
