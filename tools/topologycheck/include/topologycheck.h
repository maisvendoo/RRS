#ifndef     TOPOLOGYCHECK_H
#define     TOPOLOGYCHECK_H

#include    "cmdparser.hpp"
#include    "topology.h"
#include    "switch.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class TopologyCheck
{
public:

    TopologyCheck(){}

    ~TopologyCheck(){}

    int run(int argc, char *argv[]);

private:

    std::string route_path = "";

    Topology* topology = new Topology();

    std::vector<std::pair<dvec3, Trajectory *>> ends_without_connector = {};

    double maximum_curvature = 1.0 / 150.0;

    void configure_parser(cli::Parser &parser);

    void parse_command_line(cli::Parser &parser);

    void find_ends_without_connector(Trajectory *traj);

    void check_trajectory(Trajectory *traj);

    void check_connector_point(Switch *sw);

    void check_ends_and_point(Trajectory *traj, dvec3 point, int point_num);

    double calcCurvature(const track_t &track0, const track_t &track1);
};

#endif // TOPOLOGYCHECK_H
