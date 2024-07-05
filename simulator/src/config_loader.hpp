#ifndef FALAFELS_CONFIG_LOADER_HPP
#define FALAFELS_CONFIG_LOADER_HPP

#include <memory>
#include <unordered_map>
#include "node/node.hpp"
#include "protocol.hpp"

/**
 * Load a fried falafels deployment file.
 * Creates nodes and initialize constants.
 *
 * @param file path to the fried falafels deployment file.
 * @return A map pairing each created node pointer with its name as a key
 */
std::unordered_map<protocol::node_name, Node*> *load_config(const char* file_path);

#endif // !FALAFELS_CONFIG_LOADER_HPP
