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
#include    "zds-route-trk-struct.h"
#include    "zds-start-kilometers-dat-struct.h"
#include    "zds-speeds-dat-struct.h"
#include    "zds-svetofor-dat-struct.h"
#include    "zds-branch-tracks-dat-struct.h"
#include    "power_line_element.h"
#include    "neutral_insertion.h"
#include    "cmd-line.h"

#include    <fstream>
#include    <QTextStream>

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

    std::string         routeDir;

    zds_trajectory_data_t    tracks_data1;

    zds_trajectory_data_t    tracks_data2;

    zds_start_km_data_t start_km_data;

    zds_speeds_data_t   speeds_data1;

    zds_speeds_data_t   speeds_data2;

    zds_signals_data_t  signals_data1;

    zds_signals_data_t  signals_data2;

    zds_branch_track_data_t branch_track_data1;

    zds_branch_track_data_t branch_track_data2;

    std::vector<power_line_element_t> power_line1;

    std::vector<power_line_element_t> power_line2;

    std::vector<neutral_insertion_t> neutral_insertions;

    CmdLineParseResult parseCommandLine(int argc, char *argv[]);

    void fileToUtf8(const std::string &path);

    bool conversion(const std::string &routeDir);

    zds_track_t getNearestTrack(dvec3 point, const zds_trajectory_data_t &tracks_data, float &coord);

    bool readRouteTRK(const std::string &path, zds_trajectory_data_t &track_data);

    bool readRouteTRK(std::ifstream &stream, zds_trajectory_data_t &track_data);

    void writeProfileData(const zds_trajectory_data_t &tracks_data,
                          const std::string &file_name);

    void createPowerLine(const std::vector<zds_track_t> &tracks_data,
                         std::vector<power_line_element_t> &power_line);

    bool readStartKilometersDAT(const std::string &path, zds_start_km_data_t &waypoints);

    bool readStartKilometersDAT(QTextStream &stream, zds_start_km_data_t &waypoints);

    void writeWaypoints(const std::string &filename, const zds_start_km_data_t &waypoints);

    void writeStations(const std::string &filename, const zds_start_km_data_t &waypoints);

    bool readSpeedsDAT(const std::string &path, zds_speeds_data_t &speeds_data);

    bool readSpeedsDAT(QTextStream &stream, zds_speeds_data_t &speeds_data);

    void writeSpeeds(const std::string &filename, const zds_speeds_data_t &speeds_data);

    bool readSvetoforDAT(const std::string &path, zds_signals_data_t &signals_data);

    bool readSvetoforDAT(QTextStream &stream, zds_signals_data_t &signals_data);

    bool readBranchTracksDAT(const std::string &path, zds_branch_track_data_t &branch_data);

    bool readBranchTracksDAT(QTextStream &stream, zds_branch_track_data_t &branch_data);

    bool readRouteMAP(const std::string &path, std::vector<neutral_insertion_t> ni);

    bool findNeutralInsertions(std::ifstream &stream, std::vector<neutral_insertion_t> ni);
};

#endif // CONVERTER_H
