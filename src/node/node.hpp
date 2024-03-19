#ifndef FALAFELS_NODE_HPP
#define FALAFELS_NODE_HPP

#include <simgrid/s4u/Actor.hpp>
#include <simgrid/s4u/Engine.hpp>
#include <xbt/log.h>
#include "./roles/role.hpp"

class Node
{
    private:
        Role *role;
    public:
        Node() {}
        ~Node() {}
        void run() { this->role->run(); }
        void set_role(Role *r) { role = r; }
        Role *get_role() { return role; }
};

#endif // !FALAFELS_NODE_HPP
