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
#include "constants.hpp"
#include "utils/utils.hpp"
 
 
XBT_LOG_NEW_DEFAULT_CATEGORY(s4u_falafels_config, "Messages specific for this example");

using namespace pugi;

void node_runner(Node *node)
{
    XBT_INFO("Node: %s", node->get_role()->get_network_manager()->get_my_node_name().c_str());
    node->run();
}

/**
 * Create a network manager with the correct type.
 * @param network_manager_elem XML element that contains network manager informations.
 * @param name Name of the Node that will be associated to this network manager.
 * @return A pointer to the created NetworkManager.
 */
NetworkManager *create_network_manager(xml_node *network_manager_elem, node_name name)
{
    NetworkManager *network_manager;

    auto nm_type = network_manager_elem->attribute("type").as_string();

    if (strcmp(nm_type, "centralized") == 0)
    {
        network_manager = new CentralizedNetworkManager(name);
    }
    else if (strcmp(nm_type, "decentralized") == 0)
    {
        // TODO
        // network_manager = new DecentralizedNetworkManager(name);
    }

    XBT_INFO("With %s network manager", nm_type);

    return network_manager;
}

/**
 * Create an aggregator with the correct type.
 * @param aggregator_elem XML element that contains aggregator informations.
 * @return A pointer to the created aggregator.
 */
Aggregator *create_aggregator(xml_node *role_elem)
{
    Aggregator *role;

    xml_node args = role_elem->child("args");
    auto aggregator_type = role_elem->attribute("type").as_string();

    if (strcmp(aggregator_type, "simple") == 0)
    {
        role = new SimpleAggregator();
        XBT_INFO("With role: SimpleAggregator");
    }
    else if (strcmp(aggregator_type, "asynchronous") == 0)
    {
        float proportion_threshold = args.child("proportion_threshold").attribute("value").as_float();
        role = new AsynchronousAggregator(proportion_threshold);
        XBT_INFO("With role: AsynchronousAggregator");
        XBT_INFO("Param: proportion_threshold=%f", proportion_threshold);
    }

    return role; 
}

/**
 * Create a role to be asociated for a Node.
 * @param role_elem XML element that contains role informations.
 * @return A pointer to the created Role.
 */
Role *create_role(xml_node *role_elem)
{
    Role *role;

    if (strcmp(role_elem->name(), "aggregator") == 0)
    {
        role = create_aggregator(role_elem);
    }
    else if (strcmp(role_elem->name(), "trainer") == 0)
    {
        role = new Trainer();
        XBT_INFO("With role: Trainer");
    }
    else if (strcmp(role_elem->name(), "proxy") == 0)
    {
        // TODO
        // role = new Proxy();
        // XBT_INFO("With role: Proxy");
    }

    return role;
}

/**
 * Create a single node with its respectful configuration.
 * @param node_elem XML element that contains node informations.
 * @param name The name of the current node.
 * @return A pointer to the created Node.
 */
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

/**
 * Create nodes with their respectful configuration.
 * @param nodes_elem XML element that contains the list of nodes.
 * @return An unordered map with node_name as key and a pointer to the given Node.
 */
std::unordered_map<node_name, Node*> *create_nodes(xml_node *nodes_elem)
{
    XBT_INFO("Creating falafels nodes...");

    auto nodes_map = new std::unordered_map<node_name, Node*>();

    // Loop through each (xml) node of the document to instanciate (simulated) nodes
    for (xml_node node_elem: nodes_elem->children("node"))
    {
        node_name name = node_elem.attribute("name").as_string();
        Node *node     = create_node(&node_elem, name);

        nodes_map->insert({name, node});
    }

    // Loop a second time to set boostrap nodes
    for (xml_node node_elem: nodes_elem->children("node"))
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

    return nodes_map;
}

/**
 * Set a constant in the Constant Class
 * @param name Constant's name
 * @param value Constant's value
 */
void set_constant(const xml_attribute *name, xml_attribute *value)
{
    // If the value is empty ingore constant
    if (value->empty())
        return;

    XBT_INFO("Set %s=%s", name->as_string(), value->as_string());

    switch (str2int(name->as_string())) {
        case str2int("MODEL_SIZE_BYTES"):
            Constants::MODEL_SIZE_BYTES = value->as_int();
            break;
        case str2int("GLOBAL_MODEL_AGGREGATING_FLOPS"):
            Constants::GLOBAL_MODEL_AGGREGATING_FLOPS = value->as_double();
            break;
        case str2int("LOCAL_MODEL_TRAINING_FLOPS"):
            Constants::LOCAL_MODEL_TRAINING_FLOPS = value->as_double();
            break;
    }
}

/**
 * Loop through constant elem and set each constant.
 * @param constants_elem The element containing constant children
 */
void init_constants(xml_node *constants_elem)
{
    XBT_INFO("Initializing constants...");

    for (xml_node constant: constants_elem->children())
    {
        xml_attribute name = constant.attribute("name");
        xml_attribute value = constant.attribute("value");
        set_constant(&name, &value);
    }

    XBT_INFO("-------------------------");
}

void load_config(const char* file_path, simgrid::s4u::Engine *e)
{
    xml_document doc;
    xml_parse_result result = doc.load_file(file_path);

    xbt_assert(result != 0, "Error while loading falafels xml file");

    xml_node root_elem = doc.child("fried");
    xml_node nodes_elem = root_elem.child("nodes");
    xml_node constants_elem = root_elem.child("constants");

    init_constants(&constants_elem);

    auto nodes_map = create_nodes(&nodes_elem);  


    for (auto [name, node] : *nodes_map)
    {
        XBT_INFO("Creating actor '%s'", name.c_str());
        auto actor = simgrid::s4u::Actor::create(name, e->host_by_name(name), &node_runner, node);
    }
}
