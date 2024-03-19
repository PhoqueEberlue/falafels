#include <cstring>
#include <memory>
#include <simgrid/plugins/energy.h>
#include <simgrid/forward.h>
#include <simgrid/s4u.hpp>
#include <simgrid/s4u/Actor.hpp>
#include <simgrid/s4u/Engine.hpp>
#include <simgrid/s4u/Mailbox.hpp>
#include <string>
#include <vector>
#include <xbt/log.h>

#include "./node/node.hpp"
#include "./node/roles/aggregator.hpp"
#include "./node/roles/trainer.hpp"
#include "./node/roles/proxy.hpp"
#include "./node/network_managers/decentralized_nm.hpp"
#include "./node/network_managers/centralized_nm.hpp"
#include "./node/network_managers/nm.hpp"
#include "protocol.hpp"
#include "./utils/utils.hpp"
#include "constants.hpp"

XBT_LOG_NEW_DEFAULT_CATEGORY(s4u_falafels, "Messages specific for this example");


static void run_node(Node *node)
{
    node->run();
}

static void main_scenario(simgrid::s4u::Engine *e)
{
    // Node info definition for tremblay boostrap nodes
    auto node_info_tremblay = new NodeInfo { .name = "Tremblay", .role = NodeRole::Aggregator };
    auto node_info_jupiter = new NodeInfo { .name = "Jupiter", .role = NodeRole::Trainer };
    auto node_info_fafard = new NodeInfo { .name = "Fafard", .role = NodeRole::Trainer };

    // Defining Tremblay node
    auto bootstrap_nodes_tremblay = new std::vector<NodeInfo*>{ node_info_jupiter, node_info_fafard };
    Node *node_tremblay = new Node();

    Role *role_tremblay = new Aggregator();
    NetworkManager *network_manager_tremblay = new CentralizedNetworkManager(bootstrap_nodes_tremblay, "Tremblay");

    node_tremblay->set_role(role_tremblay);
    node_tremblay->get_role()->set_network_manager(network_manager_tremblay);

    auto actor_tremblay = simgrid::s4u::Actor::create("Tremblay", e->host_by_name("Tremblay"), &run_node, node_tremblay);

    // Defining Jupiter node
    auto bootstrap_nodes_jupiter = new std::vector<NodeInfo*> {};
    auto node_jupiter = new Node();

    auto role_jupiter = new Trainer();
    NetworkManager *network_manager_jupiter = new CentralizedNetworkManager(bootstrap_nodes_jupiter, "Jupiter");

    node_jupiter->set_role(role_jupiter);
    node_jupiter->get_role()->set_network_manager(network_manager_jupiter);

    auto actor_jupiter = simgrid::s4u::Actor::create("Jupiter", e->host_by_name("Jupiter"), &run_node, node_jupiter);

    // Defining Fafard node
    auto bootstrap_nodes_fafard = new std::vector<NodeInfo*> {};
    Node *node_fafard = new Node;

    Role *role_fafard = new Trainer();
    NetworkManager *network_manager_fafard = new CentralizedNetworkManager(bootstrap_nodes_fafard, "Fafard");

    node_fafard->set_role(role_fafard);
    node_fafard->get_role()->set_network_manager(network_manager_fafard);

    auto actor_fafard = simgrid::s4u::Actor::create("Fafard", e->host_by_name("Fafard"), &run_node, node_fafard);

}

int main(int argc, char* argv[])
{
    // Initializing host energy plugin
    sg_host_energy_plugin_init();

    simgrid::s4u::Engine e(&argc, argv);
    
    // xbt_assert(argc > 2, "Usage: %s platform_file deployment_file\n", argv[0]);

    /* Load the platform description and then deploy the application */
    e.load_platform(argv[1]);
    // e.load_deployment(argv[2]);

    main_scenario(&e);
    /* Run the simulation */
    e.run();

    XBT_INFO("Simulation is over");

    return 0;
}
