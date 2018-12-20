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

    std::vector<track_t>    tracks_data;

    CmdLineParseResult parseCommandLine(int argc, char *argv[]);
};

#endif // CONVERTER_H
