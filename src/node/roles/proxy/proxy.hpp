/* Proxy */
#ifndef FALAFELS_PROXY_HPP
#define FALAFELS_PROXY_HPP

#include "../role.hpp"

class Proxy : public Role 
{
private:
    void redirect();
public:
    Proxy();
    void run();
    NodeRole get_role_type() { return NodeRole::Proxy; };
};

#endif // !FALAFELS_PROXY_HPP
