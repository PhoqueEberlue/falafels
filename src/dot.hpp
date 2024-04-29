#ifndef DOT_HPP
#define DOT_HPP

#include <string>
#include <unordered_map>
#include <vector>

#include "utils/utils.hpp"
#include "protocol.hpp"

class DOTGenerator
{
public:
    static DOTGenerator& get_instance()
    {
        // Guaranteed to be destroyed.
        // Instantiated on first use.
        static DOTGenerator instance; 
        return instance;
    }

    DOTGenerator(DOTGenerator const&) = delete;
    void operator=(DOTGenerator const&) = delete;

    void add_to_cluster(std::string cluster_name, std::string line);
    void add_to_state(double current_time, std::string line);
    void generate_state_files();
private:
    DOTGenerator(); 
    ~DOTGenerator();

    std::unordered_map<std::string, std::vector<std::string>*> *cluster_map;
    std::unordered_map<double, std::vector<std::string>*> *dot_states;

    void display_clusters(std::ofstream &graph_file);
};

#endif //!DOT_HPP
