/* Trainer */
#ifndef FALAFELS_TRAINER_HPP
#define FALAFELS_TRAINER_HPP

#include "role.hpp"

class Trainer : public Role 
{
    private:
        void train();
    public:
        Trainer();
        void run();
        NodeRole get_role_name() { return NodeRole::Trainer; };
};

#endif //! FALAFELS_TRAINER_HPP
