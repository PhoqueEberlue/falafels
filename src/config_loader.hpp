#ifndef FALAFELS_CONFIG_LOADER_HPP
#define FALAFELS_CONFIG_LOADER_HPP

#include <unordered_map>
#include "node/node.hpp"
#include "protocol.hpp"

/**
 * Load a falafels deployment file.
 * Creates nodes and initialize constants.
 *
 * @param file_path Falafels deployment file path
 * @param e Simgrid Engine
 */
void load_config(const char* file_path, simgrid::s4u::Engine *e);

#endif // !FALAFELS_CONFIG_LOADER_HPP
