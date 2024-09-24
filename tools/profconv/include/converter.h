//------------------------------------------------------------------------------
//
//      Conversion processor
//      (c) maisvendoo, 20/12/2018
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief Conversion processor
 * \copyright maisvendoo
 * \author maisvendoo
 * \date 20/12/2018
 */

#ifndef     CONVERTER_H
#define     CONVERTER_H

#include    <vector>

//#include    "zds-profile-dat-struct.h"
#include    "zds-route-map-struct.h"
#include    "zds-route-trk-struct.h"
#include    "zds-start-kilometers-dat-struct.h"
#include    "zds-speeds-dat-struct.h"
#include    "zds-svetofor-dat-struct.h"
#include    "zds-branch-tracks-dat-struct.h"
#include    "trajectory_struct.h"
#include    "power_line_element.h"
#include    "neutral_insertion.h"
#include    "cmd-line.h"

#include    <fstream>
#include    <QTextStream>

#define     DIR_TOPOLOGY     std::string("topology")
#define     DIR_TRAJECTORIES std::string("trajectories")
#define     DIR_ALSN_MAP     std::string("trajectory-ALSN")
#define     DIR_SPEEDMAP     std::string("trajectory-speedmap")
#define     FILE_TOPOLOGY    std::string("topology.xml")
#define     FILE_START_POINT std::string("waypoints.conf")
#define     FILE_STATIONS    std::string("stations.conf")
#define     FILE_SPEEDMAP    std::string("trajectory-speedmap.xml")
#define     FILE_ALSN        std::string("trajectory-ALSN.xml")
#define     FILE_ALSN_25HZ   std::string("ALSN_25Hz.xml")
#define     FILE_ALSN_50HZ   std::string("ALSN_50Hz.xml")
#define     FILE_TRAJ_EXTENTION   std::string(".traj")
#define     FILE_BACKUP_EXTENTION std::string(".bak")
#define     FILE_BACKUP_PREFIX    std::string("~")
#define     CONFIGNODE_TRAJ_2LVL  std::string("Trajectories")
#define     CONFIGNODE_TRAJ_3LVL  std::string("Name")
#define     DELIMITER_SYMBOL char('\t')
#define     ADD_ZDS_TRACK_NUMBER_TO_FILENAME bool(true)

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class ZDSimConverter
{
public:

    ZDSimConverter();

    ~ZDSimConverter();

    int run(int argc, char *argv[]);

private:

    size_t railway_coord_sections1 = 0;

    size_t railway_coord_sections2 = 0;

    std::string         routeDir;

    std::string         topologyDir;

    std::string         trajectoriesDir;

    std::string         ALSN_Dir;

    std::string         speedmapDir;

    zds_route_map_data_t    route_map_data;

    zds_trajectory_data_t   tracks_data1;

    zds_trajectory_data_t   tracks_data2;

    zds_start_km_data_t start_km_data;

    zds_speeds_data_t   speeds_data1;

    zds_speeds_data_t   speeds_data2;

    zds_signals_data_t  signals_data1;

    zds_signals_data_t  signals_data2;

    zds_signals_data_t  signals_reverse_data1;

    zds_signals_data_t  signals_reverse_data2;

    zds_branch_track_data_t branch_track_data1;

    zds_branch_track_data_t branch_track_data2;

    zds_branch_2_2_data_t branch_2minus2_data;

    zds_branch_2_2_data_t branch_2plus2_data;

    route_trajectories_t trajectories1;

    route_trajectories_t trajectories2;

    route_connectors_t split_data1;

    route_connectors_t split_data2;

    route_connectors_t branch_connectors;

    start_point_data_t start_points;

    speedmap_data_t   speedmap_data;

    std::vector<power_line_element_t> power_line1;

    std::vector<power_line_element_t> power_line2;

    std::vector<neutral_insertion_t> neutral_insertions;

    CmdLineParseResult parseCommandLine(int argc, char *argv[]);

    QString fileToQString(const std::string &path);

    bool conversion(const std::string &routeDir);

    zds_track_t getNearestTrack(dvec3 point, const zds_trajectory_data_t &tracks_data, float &coord);

    bool readRouteMAP(const std::string &path, zds_route_map_data_t &map_data);

    bool readRouteMAP(QTextStream &stream, zds_route_map_data_t &map_data);

    bool readRouteTRK(const std::string &path, zds_trajectory_data_t &track_data, const int &dir);

    bool readRouteTRK(std::ifstream &stream, zds_trajectory_data_t &track_data, const int &dir);

    void writeProfileData(const zds_trajectory_data_t &tracks_data,
                          const std::string &file_name);

    void writeMainTrajectory(const std::string &filename, const zds_trajectory_data_t &tracks_data);

    void createPowerLine(const std::vector<zds_track_t> &tracks_data,
                         std::vector<power_line_element_t> &power_line);

    bool findNeutralInsertions(std::vector<neutral_insertion_t> ni);

    bool readStartKilometersDAT(const std::string &path, zds_start_km_data_t &waypoints);

    bool readStartKilometersDAT(QTextStream &stream, zds_start_km_data_t &waypoints);

    void writeStationsOld(const std::string &filename, const zds_start_km_data_t &waypoints);

    bool readSpeedsDAT(const std::string &path, zds_speeds_data_t &speeds_data, const int &dir);

    bool readSpeedsDAT(QTextStream &stream, zds_speeds_data_t &speeds_data, const int &dir);

    void writeOldSpeeds(const std::string &filename, const zds_speeds_data_t &speeds_data);

    bool readSvetoforDAT(const std::string &path, zds_signals_data_t &signals_data, const int &dir);

    bool readSvetoforDAT(QTextStream &stream, zds_signals_data_t &signals_data, const int &dir);

    bool readBranchTracksDAT(const std::string &path, const int &dir);

    bool readBranchTracksDAT(QTextStream &stream, const int &dir);

    bool checkIsToOtherMain(zds_branch_point_t* branch_point, bool is_add_2minus2);

    void findFromOtherMain(zds_branch_point_t* branch_point);

    bool calcBranchTrack1(zds_branch_track_t* branch_track);

    bool calcBranchTrack2(zds_branch_track_t* branch_track);

    void calcBranch22(zds_branch_2_2_t* branch22);

    void findSplitsMainTrajectories(const int &dir);

    void addOrCreateSplit(route_connectors_t &split_data, const split_zds_trajectory_t &split_point);

    void splitMainTrajectory(const int &dir);

    void splitAndNameBranch(zds_branch_track_t* branch_track, const int &dir, size_t num_trajectories);

    void nameBranch22(zds_branch_2_2_t* branch_track, const int &dir, size_t num_trajectories);

    void findStartPointsBySignals(const route_connectors_t &connectors);

    bool createSpeedMap();

    void writeSplits(const route_connectors_t &connectors, const int &dir);

    void writeTopologyTrajectory(const trajectory_t* trajectory);

    void writeTopologyConnectors();

    void writeStartPoints(const start_point_data_t &start_points);

    void writeStations(const zds_start_km_data_t &waypoints);

    void writeSpeedmap_OLD();

    void writeALSN_OLD();

    void writeSpeedmap();

    void writeALSN();
};

#endif // CONVERTER_H
