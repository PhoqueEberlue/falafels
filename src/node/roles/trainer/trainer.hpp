/* Trainer */
#ifndef FALAFELS_TRAINER_HPP
#define FALAFELS_TRAINER_HPP

#include "../role.hpp"

class Trainer : public Role 
{
    private:
        void train();
        void send_local_model(node_name dest);
    public:
        Trainer();
        void run();
        NodeRole get_role_type() { return NodeRole::Trainer; };
};

#endif //! FALAFELS_TRAINER_HPP
