#include <cstring>
#include <vector>
#include <xbt/asserts.h>
#include <xbt/log.h>

#include "node/network_managers/nm.hpp"
#include "node/roles/aggregator/asynchronous_aggregator.hpp"
#include "node/roles/aggregator/simple_aggregator.hpp"
#include "node/roles/trainer/trainer.hpp"
#include "node/roles/proxy/proxy.hpp"
#include "node/network_managers/centralized_nm.hpp"
#include "node/network_managers/decentralized_nm.hpp"
#include "config_loader.hpp"
#include "../pugixml/pugixml.hpp"
 
 
XBT_LOG_NEW_DEFAULT_CATEGORY(s4u_falafels_config, "Messages specific for this example");

using namespace pugi;

void node_runner(Node *node)
{
    XBT_INFO("Node: %s", node->get_role()->get_network_manager()->get_my_node_name().c_str());
    node->run();
}

NetworkManager *create_network_manager(xml_node *network_manager_elem, node_name name)
{
    NetworkManager *network_manager;

    xml_node nm_type = network_manager_elem->first_child();

    if (strcmp(nm_type.name(), "centralized") == 0)
    {
        network_manager = new CentralizedNetworkManager(name);
    }
    else if (strcmp(nm_type.name(), "decentralized") == 0)
    {
        // TODO
        // network_manager = new DecentralizedNetworkManager(name);
    }

    XBT_INFO("With %s network manager", nm_type.name());

    return network_manager;
}

Aggregator *create_aggregator(xml_node *aggregator_elem)
{
    Aggregator *role;

    if (strcmp(aggregator_elem->name(), "simple") == 0)
    {
        role = new SimpleAggregator();
        XBT_INFO("With role: SimpleAggregator");
    }
    else if (strcmp(aggregator_elem->name(), "asynchronous") == 0)
    {
        role = new AsynchronousAggregator();
        XBT_INFO("With role: Asynchronous");
    }

    return role; 
}

Role *create_role(xml_node *role_elem)
{
    Role *role;

    xml_node role_type_elem = role_elem->first_child();

    if (strcmp(role_type_elem.name(), "aggregator") == 0)
    {
        xml_node aggregator_elem = role_type_elem.first_child();
        role = create_aggregator(&aggregator_elem);
    }
    else if (strcmp(role_type_elem.name(), "trainer") == 0)
    {
        role = new Trainer();
        XBT_INFO("With role: Trainer");
    }
    else if (strcmp(role_type_elem.name(), "proxy") == 0)
    {
        // TODO
        // role = new Proxy();
        // XBT_INFO("With role: Proxy");
    }

    return role;
}

Node *create_node(xml_node *node_elem, node_name name)
{
    XBT_INFO("------------------------------");
    XBT_INFO("Creating node: %s", name.c_str());

    xml_node role_elem = node_elem->first_child();
    Role *role = create_role(&role_elem);

    xml_node network_manager_elem = role_elem.next_sibling(); 
    NetworkManager *network_manager = create_network_manager(&network_manager_elem, name);

    // Returning new falafels node
    return new Node(role, network_manager);
}

/* Load falafels deployment file */
void load_config(const char* file_path, simgrid::s4u::Engine *e)
{
    xml_document doc;
    xml_parse_result result = doc.load_file(file_path);

    xbt_assert(result != 0, "Error while loading falafels xml file");

    xml_node root           = doc.child("deployment");
    auto nodes_map          = new std::unordered_map<node_name, Node*>();

    // Loop through each (xml) node of the document to instanciate (simulated) nodes
    for (xml_node node_elem: root.children("node"))
    {
        node_name name = node_elem.attribute("name").as_string();
        Node *node     = create_node(&node_elem, name);

        nodes_map->insert({name, node});
    }

    // Loop a second time to set boostrap nodes
    for (xml_node node_elem: root.children("node"))
    {
        node_name name = node_elem.attribute("name").as_string();
        auto bootstrap_nodes = new std::vector<NodeInfo*>(); 

        // Loop through bootstrap nodes
        for (xml_node bootstrap_node: node_elem.child("network-manager").child("bootstrap-nodes").children())
        {
            // Get corresponding node info
            XBT_INFO("Fetching NodeInfo of '%s' to be added as boostrap node for '%s'", bootstrap_node.text().as_string(), name.c_str());
            NodeInfo *node_info = nodes_map->at(bootstrap_node.text().as_string())->get_node_info();

            bootstrap_nodes->push_back(node_info);
        }

        // Set boostrap nodes
        nodes_map->at(name)->get_role()->get_network_manager()->set_bootstrap_nodes(bootstrap_nodes); 
    } 

    for (auto [name, node] : *nodes_map)
    {
        XBT_INFO("Creating actor '%s'", name.c_str());
        auto actor = simgrid::s4u::Actor::create(name, e->host_by_name(name), &node_runner, node);
    }
}

