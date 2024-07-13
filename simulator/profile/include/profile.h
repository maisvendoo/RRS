//------------------------------------------------------------------------------
//
//
//
//
//------------------------------------------------------------------------------
#ifndef     PROFILE_H
#define     PROFILE_H

#include    <string>
#include    <fstream>
#include    <vector>

#include    "profile-point.h"
#include    "zds-track.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class Profile
{
public:

    Profile()
        : is_ready(false)
    {

    }

    Profile(int dir, const std::string &routeDir);

    ~Profile();

    bool isReady() const;

    profile_point_t getElement(double railway_coord, int dir);

private:

    bool    is_ready;

    std::vector<zds_track_t> profile_data;

    bool load(const std::string &path);

    bool load(std::ifstream &stream);
};

#endif // PROFILE_H
