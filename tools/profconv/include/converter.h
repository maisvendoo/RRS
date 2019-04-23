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

#include    "track.h"
#include    "waypoint.h"
#include    "power_line_element.h"
#include    "neutral_insertion.h"
#include    "cmd-line.h"

#include    <fstream>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class ProfConverter
{
public:

    ProfConverter();

    ~ProfConverter();

    int run(int argc, char *argv[]);

private:

    std::string             routeDir;

    std::vector<track_t>    tracks_data1;

    std::vector<track_t>    tracks_data2;

    std::vector<waypoint_t> waypoints;

    std::vector<power_line_element_t> power_line1;

    std::vector<power_line_element_t> power_line2;

    std::vector<neutral_inserion_t> neutral_insertions;

    CmdLineParseResult parseCommandLine(int argc, char *argv[]);

    bool load(const std::string &path, std::vector<track_t> &track_data);

    bool load(std::ifstream &stream, std::vector<track_t> &track_data);

    bool readWaypoints(const std::string &path, std::vector<waypoint_t> &waypoints);

    bool readWaypoints(std::wifstream &stream, std::vector<waypoint_t> &waypoints);

    void writeWaypoints(const std::string &filename, std::vector<waypoint_t> &waypoints);

    bool conversion(const std::string &routeDir);

    void writeProfileData(const std::vector<track_t> &tracks_data,
                          const std::string &file_name);

    void fileToUtf8(const std::string &path);    

    track_t getNearestTrack(Vec3 point, const std::vector<track_t> &tracks_data, float &coord);

    void createPowerLine(const std::vector<track_t> &tracks_data,
                         std::vector<power_line_element_t> &power_line);


    bool loadNeutralInsertions(const std::string &path, std::vector<neutral_inserion_t> ni);

    bool loadNeutralInsertions(std::ifstream &stream, std::vector<neutral_inserion_t> ni);
};

#endif // CONVERTER_H
