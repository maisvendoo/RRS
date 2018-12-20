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

    CmdLineParseResult parseCommandLine(int argc, char *argv[]);

    bool load(const std::string &path, std::vector<track_t> &track_data);

    bool load(std::ifstream &stream, std::vector<track_t> &track_data);

    bool conversion(const std::string &routeDir);

    void writeProfileData(const std::vector<track_t> &tracks_data,
                          const std::string &file_name);
};

#endif // CONVERTER_H
