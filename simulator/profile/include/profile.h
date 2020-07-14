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

#include    "profile-element.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class Profile
{
public:

    Profile()
        : is_ready(false)
        , dir(1)
    {

    }

    Profile(int dir, const std::string &routeDir);

    ~Profile();

    bool isReady() const;

    profile_element_t getElement(double railway_coord);

private:

    bool    is_ready;

    int     dir;

    std::vector<profile_element_t> profile_data;

    bool load(const std::string &path);

    bool load(std::ifstream &stream);
};

#endif // PROFILE_H
