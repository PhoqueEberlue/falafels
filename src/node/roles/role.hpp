/* Role */
#ifndef FALAFELS_ROLE_HPP
#define FALAFELS_ROLE_HPP

#include "../network_managers/nm.hpp"

class Role
{
    private:
        NetworkManager *network_manager;
    public:
        Role(){}
        virtual ~Role(){}
        virtual void run() = 0;
        virtual NodeRole get_role_type() = 0;
        void set_network_manager(NetworkManager *nm) { network_manager = nm; }
        NetworkManager *get_network_manager() { return network_manager; }
};

#endif // !FALAFELS_ROLE_HPP
