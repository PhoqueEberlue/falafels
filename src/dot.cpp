#include "dot.hpp"
#include "protocol.hpp"
#include <format>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <xbt/log.h>

using namespace std;

XBT_LOG_NEW_DEFAULT_CATEGORY(s4u_dot, "Messages specific for this example");

DOTGenerator::DOTGenerator() 
{
    this->dot_states = new unordered_map<double, vector<string>*>();
    this->cluster_map = new unordered_map<string, vector<string>*>();
}

DOTGenerator::~DOTGenerator()
{
    for (auto [_, vec]: *this->dot_states) { delete vec; }
    for (auto [_, vec]: *this->cluster_map) { delete vec; }

    delete this->dot_states;
    delete this->cluster_map;
}

void DOTGenerator::add_to_cluster(string cluster_name, string line)
{
    if (this->cluster_map->contains(cluster_name))
    {
        this->cluster_map->at(cluster_name)->push_back(line);
    }
    else 
    {
        this->cluster_map->insert({ cluster_name, new vector<string>({line}) });
    }
}

void DOTGenerator::add_to_state(double current_time, string line)
{
    if (this->dot_states->contains(current_time))
    {
        this->dot_states->at(current_time)->push_back(line);
    }
    else 
    {
        this->dot_states->insert({ current_time, new vector<string>({line}) });
    }
}

string fill_zeros(string str)
{
    uint8_t nb_to_fill = 18 - str.size();

    for (int i = 0; i < nb_to_fill; i++)
    {
        str.push_back('0');
    }
    
    return str;
}

void DOTGenerator::display_clusters(ofstream &graph_file)
{
    for (auto [cluster_name, lines]: *cluster_map)
    {
        graph_file << "\tsubgraph {\n";
        graph_file << "\t\tedge [style=\"invis\"]\n";

        for (auto line: *lines)
        {
            graph_file << std::format("\t\t{};\n", line);
        }

        graph_file << "\t}\n";
    }
}
void DOTGenerator::generate_state_files()
{
    for (auto [time, links] : *this->dot_states)
    {
        std::ofstream graph_file;

        graph_file.open(std::format("./dot-files/{}.dot", fill_zeros(std::to_string(time))));

        graph_file << "digraph my_graph {\n";
        graph_file << "\tK=2.5\n";
        graph_file << "\tsize=5\n";
        graph_file << "\tratio=fill\n";

        this->display_clusters(graph_file);

        // Add packet events 
        for (auto event : *links)
        {
            // XBT_INFO("[%f]: %s", time, event.c_str());
            graph_file << std::format("\t{}\n", event);
        }

        graph_file << "}";

        graph_file.close();
    }
}
