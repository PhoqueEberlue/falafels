#include <simgrid/plugins/energy.h>
#include <simgrid/forward.h>
#include <simgrid/s4u.hpp>
#include <simgrid/s4u/Actor.hpp>
#include <simgrid/s4u/Engine.hpp>
#include <simgrid/s4u/Mailbox.hpp>
#include <string>
#include <xbt/log.h>

#include "config_loader.hpp"
#include "dot.hpp"
#include "node/node.hpp"

XBT_LOG_NEW_DEFAULT_CATEGORY(s4u_main, "Messages specific for this example");

int main(int argc, char* argv[])
{
    simgrid::s4u::Engine e(&argc, argv);

    // Initializing host and link energy plugins
    sg_host_energy_plugin_init();
    sg_link_energy_plugin_init();

    xbt_assert(argc > 2, "Usage: %s platform_file deployment_file\n", argv[0]);

    /* Load the platform description and then deploy the application */
    e.load_platform(argv[1]);

    // Using our own deployment function instead of simgrid's one
    // e.load_deployment(argv[2]);

    auto nodes_map = load_config(argv[2]); 

    for (auto [name, node] : *nodes_map)
    {
        XBT_INFO("Initializing node '%s'", name.c_str());
        node->run();
        delete node;
    }

    /* Run the simulation */
    e.run();

    // DOTGenerator::get_instance().generate_state_files();

    delete nodes_map;

    XBT_INFO("Simulation is over");

    return 0;
}
