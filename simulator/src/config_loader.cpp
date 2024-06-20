#include <cstring>
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>
#include <xbt/asserts.h>
#include <xbt/log.h>
#include <pugixml.hpp>

#include "node/network_managers/nm.hpp"
#include "node/roles/aggregator/asynchronous_aggregator.hpp"
#include "node/roles/aggregator/hierarchical_aggregator.hpp"
#include "node/roles/aggregator/simple_aggregator.hpp"
#include "node/roles/trainer/trainer.hpp"
// #include "node/roles/proxy/proxy.hpp"
#include "node/network_managers/star_nm.hpp"
#include "node/network_managers/ring_nm.hpp"
#include "node/network_managers/full_nm.hpp"
#include "config_loader.hpp"
#include "constants.hpp"
#include "protocol.hpp"
#include "utils/utils.hpp"
 
using namespace pugi;
using namespace std;
 
XBT_LOG_NEW_DEFAULT_CATEGORY(s4u_falafels_config, "Messages specific for this example");

/**
 * Parse arguments in the form of a list of args elements such as: <arg name="" value=""/>
 * @param iterator over the arg elements
 * @return A pointer to a unordered map containing key, values as strings.
 */
unordered_map<string, string> *parse_arguments(xml_object_range<xml_node_iterator> *args)
{
    auto res = new unordered_map<string, string>();

    for (auto arg: *args)
    {
        res->insert({arg.attribute("name").as_string(), arg.attribute("value").as_string()});
    }
    
    return res;
}

/**
 * Create a network manager with the correct type.
 * @param network_manager_elem XML element that contains network manager informations.
 * @param node_info NodeInfo of the Node that will be associated to this network manager.
 * @param topology Topology used in the Node's network.
 * @return A pointer to the created NetworkManager.
 */
NetworkManager *create_network_manager(xml_node *network_manager_elem, NodeInfo node_info, string topology)
{
    NetworkManager *network_manager;

    // Topology of the cluster implies the NM type
    auto nm_type = topology.c_str();

    if (strcmp(nm_type, "star") == 0)
    {
        network_manager = new StarNetworkManager(node_info);
    }
    else if (strcmp(nm_type, "ring") == 0)
    {
        network_manager = new RingNetworkManager(node_info);
    }
    else if (strcmp(nm_type, "full") == 0)
    {
        network_manager = new FullyConnectedNetworkManager(node_info);
    }

    XBT_INFO("With %s network manager", nm_type);

    return network_manager;
}

/**
 * Create an aggregator with the correct type.
 * @param aggregator_elem XML element that contains aggregator informations.
 * @param args aggregator's arguments already parsed.
 * @return A pointer to the created aggregator.
 */
Aggregator *create_aggregator(xml_node *role_elem, unordered_map<string, string> *args, node_name name)
{
    Aggregator *aggregator;
    auto aggregator_type = role_elem->attribute("type").as_string();

    if (strcmp(aggregator_type, "simple") == 0)
    {
        XBT_INFO("With role: SimpleAggregator");
        aggregator = new SimpleAggregator(args, name);
    }
    else if (strcmp(aggregator_type, "asynchronous") == 0)
    {
        XBT_INFO("With role: AsynchronousAggregator");
        aggregator = new AsynchronousAggregator(args, name);
    }
    else if (strcmp(aggregator_type, "hierarchical") == 0)
    {
        XBT_INFO("With role: Hierarchical");
        aggregator = new HierarchicalAggregator(args, name);
    }

    return aggregator; 
}

/**
 * Create a role to be asociated for a Node.
 * @param role_elem XML element that contains role informations.
 * @return A pointer to the created Role.
 */
Role *create_role(xml_node *role_elem, node_name name)
{
    Role *role;

    auto args_iter = role_elem->children();
    auto args = parse_arguments(&args_iter);

    if (strcmp(role_elem->name(), "aggregator") == 0)
    {
        role = create_aggregator(role_elem, args, name);
    }
    else if (strcmp(role_elem->name(), "trainer") == 0)
    {
        role = new Trainer(args, name);
        XBT_INFO("With role: Trainer");
    }
    else if (strcmp(role_elem->name(), "proxy") == 0)
    {
        // TODO
        // role = new Proxy(args);
        // XBT_INFO("With role: Proxy");
    }

    return role;
}

/**
 * Create a single node with its respectful configuration.
 * @param node_elem XML element that contains node informations.
 * @param name The name of the current node.
 * @param topology Topology used in the Node's network.
 * @return A pointer to the created Node.
 */
Node *create_node(xml_node *node_elem, node_name name, string topology)
{
    XBT_INFO("------------------------------");
    XBT_INFO("Creating node: %s", name.c_str());

    xml_node role_elem = node_elem->first_child();
    Role *role = create_role(&role_elem, name);

    NodeInfo node_info = NodeInfo { .name = name, .role=role->get_role_type() };

    xml_node network_manager_elem = role_elem.next_sibling(); 
    auto network_manager = create_network_manager(&network_manager_elem, node_info, topology);

    // Returning new falafels node
    return new Node(role, network_manager);
}

/**
 * Create nodes with their respectful configuration and updates the unordered map.
 * @param An unordered map with node_name as key and a pointer to the given Node.
 * @param nodes_elem XML element that contains the list of nodes.
 */
void create_nodes(unordered_map<node_name, Node*> *nodes_map, xml_node *nodes_elem)
{
    XBT_INFO("Creating falafels nodes...");

    string topology = nodes_elem->attribute("topology").as_string();

    // Loop through each (xml) node of the document to instanciate (simulated) nodes
    for (xml_node node_elem: nodes_elem->children("node"))
    {
        node_name name = node_elem.attribute("name").as_string();
        Node *node = create_node(&node_elem, name, topology);

        nodes_map->insert({name, node});
    }

    // Loop a second time to set boostrap nodes
    for (xml_node node_elem: nodes_elem->children("node"))
    {
        node_name name = node_elem.attribute("name").as_string();
        auto bootstrap_nodes = new vector<NodeInfo>(); 

        // Loop through bootstrap nodes
        for (xml_node arg: node_elem.child("network-manager").children())
        {
            // If argument name is bootstrap_node
            if (strcmp(arg.attribute("name").as_string(), "bootstrap-node") == 0)
            {
                // Getting value of the argument
                auto bootstrap_node = arg.attribute("value").as_string();
                // Get corresponding node info
                XBT_INFO("Fetching NodeInfo of '%s' to be added as boostrap node for '%s'", bootstrap_node, name.c_str());
                NodeInfo node_info = nodes_map->at(bootstrap_node)->get_node_info();

                bootstrap_nodes->push_back(node_info);
            }
        }

        // Set boostrap nodes
        nodes_map->at(name)->set_bootstrap_nodes(bootstrap_nodes); 
    }
}

/**
 * Set a constant in the Constant Class.
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
        case str2int("REGISTRATION_TIMEOUT"):
            Constants::REGISTRATION_TIMEOUT = value->as_double();
            break;
        case str2int("END_CONDITION_DURATION_TRAINING_PHASE"):
            Constants::END_CONDITION_DURATION_TRAINING_PHASE = value->as_double();
            break;
        case str2int("END_CONDITION_NUMBER_GLOBAL_EPOCHS"):
            Constants::END_CONDITION_NUMBER_GLOBAL_EPOCHS = value->as_double();
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

/**
 * Loads a fried falafels deployment file.
 * @param file path to the fried falafels deployment file.
 * @return A map pairing each created node pointer with its name as a key
 */
unordered_map<node_name, Node*> *load_config(const char* file_path)
{
    xml_document doc;
    xml_parse_result result = doc.load_file(file_path);

    xbt_assert(result != 0, "Error while loading fried falafels deployment file");

    xml_node root_elem = doc.child("fried");
    xml_node constants_elem = root_elem.child("constants");
    xml_node clusters_elem = root_elem.child("clusters");

    init_constants(&constants_elem);

    auto nodes_map = new unordered_map<node_name, Node*>();

    for (auto cluster: clusters_elem.children())
    {
        create_nodes(nodes_map, &cluster);  
    }
    return nodes_map; 
}
