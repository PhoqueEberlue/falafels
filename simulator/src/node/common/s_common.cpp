#include "./s_common.hpp"
#include <simgrid/s4u/Engine.hpp>

double SCommon::get_time()
{
    return simgrid::s4u::Engine::get_instance()->get_clock();
}

void SCommon::kill_processes()
{
    // TODO
    // std::string my_node_name = this->my_node_info.name;

    // // Ignore because hierarchical_nm doesn't have any associated role
    // if (my_node_name.contains("hierarchical_")) return;

    // // Get the actors running on the current host
    // auto actors = simgrid::s4u::Engine::get_instance()->host_by_name(my_node_name)->get_all_actors();

    // for (auto actor : actors)
    // {
    //     // Delete the actor representing the Role process
    //     if (actor->get_name().compare(std::format("{}_role", my_node_name)) == 0)
    //     {
    //         XBT_INFO("Killing actor: %s", actor->get_name().c_str());
    //         actor->kill();
    //     }
    // }
}
