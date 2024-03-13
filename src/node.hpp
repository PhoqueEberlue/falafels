#ifndef FALAFELS_NODE_HPP
#define FALAFELS_NODE_HPP

#include <simgrid/s4u/Actor.hpp>
#include <simgrid/s4u/Engine.hpp>
#include <simgrid/s4u/Mailbox.hpp>
#include <xbt/log.h>

// Abstract class node
class Node
{
    public:
        virtual void run() = 0;
        
        simgrid::s4u::Mailbox* mailbox;
        std::vector<std::string> start_nodes;
        std::string host_name;
};

class Aggregator : Node
{
    private:
        void aggregate();
        void send_global_model();
        void wait_local_models();
        void send_kills();
    public:
        Aggregator(std::string host_name, std::vector<std::string> start_nodes);
        void run();
};

class Trainer : Node
{
    private:
        void train();
    public:
        Trainer(std::string host_name, std::vector<std::string> start_nodes);
        void run();
};

class Proxy : Node
{
    private:
        void redirect();
    public:
        Proxy(std::string host_name, std::vector<std::string> start_nodes);
        void run();
};

#endif // !FALAFELS_NODE_HPP
