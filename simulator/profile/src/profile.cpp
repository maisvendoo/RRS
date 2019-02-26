#include    "profile.h"

#include    <iostream>
#include    <sstream>

#include    "filesystem.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Profile::Profile(int dir, const std::string &routeDir)
    : is_ready(false)
    , dir(dir)
{
    FileSystem &fs = FileSystem::getInstance();
    std::string path = fs.toNativeSeparators(routeDir);

    if (dir > 0)
    {
        path = fs.combinePath(path, "profile1.conf");
    }
    else
    {
        path = fs.combinePath(path, "profile2.conf");
    }

    is_ready = load(path);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Profile::~Profile()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Profile::isReady() const
{
    return is_ready;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
profile_element_t Profile::getElement(double railway_coord)
{
    if (profile_data.size() == 0)
        return profile_element_t();

    if (railway_coord < (*profile_data.begin()).railway_coord)
        return profile_element_t();

    if (railway_coord >= (*(profile_data.end() - 1)).railway_coord)
        return profile_element_t();

    profile_element_t profile_element;

    size_t left_idx = 0;
    size_t right_idx = profile_data.size() - 1;
    size_t idx = (left_idx + right_idx) / 2;

    while (idx != left_idx)
    {
        profile_element = profile_data.at(idx);

        if (railway_coord <= profile_element.railway_coord)
            right_idx = idx;
        else
            left_idx = idx;

        idx = (left_idx + right_idx) / 2;
    }

    return profile_data.at(idx);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Profile::load(const std::string &path)
{
    if (path.empty())
        return false;

    std::ifstream stream(path.c_str(), std::ios::in);

    if (!stream.is_open())
    {
        std::cout << "File " << path << " not opened" << std::endl;
        return false;
    }

    return load(stream);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::string delete_symbol(const std::string &str, char symbol)
{
    std::string tmp = str;
    tmp.erase(std::remove(tmp.begin(), tmp.end(), symbol), tmp.end());
    return tmp;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Profile::load(std::ifstream &stream)
{
    while (!stream.eof())
    {
        std::string line = "";
        std::getline(stream, line);

        if (line.empty())
            continue;

        std::string tmp = delete_symbol(line, '\r');

        profile_element_t profile_element;
        std::istringstream ss(tmp);

        ss >> profile_element.railway_coord
           >> profile_element.inclination
           >> profile_element.curvature;

        profile_element.railway_coord *= 1000.0;

        profile_data.push_back(profile_element);
    }

    return true;
}
