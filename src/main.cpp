#include <cstdlib>
#include <cstring>
#include <simgrid/plugins/energy.h>
#include <simgrid/forward.h>
#include <simgrid/s4u.hpp>
#include <simgrid/s4u/Actor.hpp>
#include <simgrid/s4u/Engine.hpp>
#include <simgrid/s4u/Mailbox.hpp>
#include <string>
#include <vector>
#include <xbt/log.h>

#include "node.hpp"
#include "utils.hpp"
#include "constants.hpp"

XBT_LOG_NEW_DEFAULT_CATEGORY(s4u_falafels, "Messages specific for this example");

static void node(std::vector<std::string> args)
{
    auto host_name = simgrid::s4u::this_actor::get_host()->get_cname();

    // Index 0 of args is the current function name
    auto node_type = args[1];
    auto start_nodes_plain = args[2];

    XBT_INFO("---------- Launching new node ----------");
    XBT_INFO("Host name  : %s", host_name);
    XBT_INFO("Node type  : %s", node_type.c_str());
    XBT_INFO("Start nodes: %s", start_nodes_plain.c_str());
    XBT_INFO("----------------------------------------");

    // start_nodes.
    auto start_nodes_vec = split_string(start_nodes_plain, ' ');

    if (strcmp(node_type.c_str(), "aggregator") == 0)
    {
        auto node = std::make_unique<Aggregator>(host_name, start_nodes_vec);
        node->run();
    } 
    else if (strcmp(node_type.c_str(), "trainer") == 0)
    {
        auto node = std::make_unique<Trainer>(host_name, start_nodes_vec);
        node->run();
    }
}

int main(int argc, char* argv[])
{
    // Initializing host energy plugin
    sg_host_energy_plugin_init();

    simgrid::s4u::Engine e(&argc, argv);
    
    xbt_assert(argc > 2, "Usage: %s platform_file deployment_file\n", argv[0]);

    /* Register the function to launch the nodes */
    e.register_function("node", &node);

    /* Load the platform description and then deploy the application */
    e.load_platform(argv[1]);
    e.load_deployment(argv[2]);

    /* Run the simulation */
    e.run();

    XBT_INFO("Simulation is over");

    return 0;
}
