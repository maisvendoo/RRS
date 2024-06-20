#include    "profile.h"

#include    <iostream>
#include    <sstream>

#include    "filesystem.h"
#include    "Journal.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Profile::Profile(int dir, const std::string &routeDir)
    : is_ready(false)
{
    FileSystem &fs = FileSystem::getInstance();
    std::string path = fs.toNativeSeparators(routeDir);

    Journal::instance()->info("Route directory: " + QString(routeDir.c_str()));

    if (dir > 0)
    {
        path = fs.combinePath(path, "route1.trk");
        Journal::instance()->info("Direction: forward (route1.trk)");
    }
    else
    {
        path = fs.combinePath(path, "route2.trk");
        Journal::instance()->info("Direction: backward (route2.trk)");
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
profile_point_t Profile::getElement(double railway_coord, int dir)
{
    if (profile_data.size() == 0)
        return profile_point_t();

    if (railway_coord < (*profile_data.begin()).railway_coord)
    {
        profile_point_t profile_point = profile_point_t();
        zds_track_t track = *profile_data.begin();
        double motion = railway_coord - track.railway_coord;
        dvec3 horizontal_orth = normalize(dvec3(track.orth.x, track.orth.y, 0.0));

        profile_point.ordinate = track.ordinate + motion;
        profile_point.position = track.begin_point + horizontal_orth * motion;
        profile_point.orth = horizontal_orth * static_cast<double>(dir);
        profile_point.right = track.right;
        profile_point.up = track.up;
        return profile_point;
    }

    if ( railway_coord >= ( (*(profile_data.end() - 1)).railway_coord + (*(profile_data.end() - 1)).length ) )
    {
        profile_point_t profile_point = profile_point_t();
        zds_track_t track = *(profile_data.end() - 1);
        double motion = railway_coord - track.railway_coord;
        dvec3 horizontal_orth = normalize(dvec3(track.orth.x, track.orth.y, 0.0));

        profile_point.ordinate = track.ordinate + motion;
        profile_point.position = track.begin_point + horizontal_orth * motion;
        profile_point.orth = horizontal_orth * static_cast<double>(dir);
        profile_point.right = track.right;
        profile_point.up = track.up;
        return profile_point;
    }

    zds_track_t track = zds_track_t();
    size_t left_idx = 0;
    size_t right_idx = profile_data.size() - 1;
    size_t idx = (left_idx + right_idx) / 2;

    while (idx != left_idx)
    {
        track = profile_data.at(idx);

        if (railway_coord <= track.railway_coord)
            right_idx = idx;
        else
            left_idx = idx;

        idx = (left_idx + right_idx) / 2;
    }
    track = profile_data.at(idx);

    profile_point_t profile_point;
    double motion = railway_coord - track.railway_coord;
    double relative_motion = motion / track.length;

    profile_point.ordinate = track.ordinate + motion;
    profile_point.inclination = track.inclination * static_cast<double>(dir);
    profile_point.curvature = track.curvature;
    profile_point.position = track.begin_point + track.orth * motion;
    if ((relative_motion < 0.5) && (idx > 0))
    {
        zds_track_t prev_track = profile_data.at(idx - 1);
        profile_point.orth = track.orth * (0.5 + relative_motion) * static_cast<double>(dir);
        profile_point.orth += prev_track.orth * (0.5 - relative_motion) * static_cast<double>(dir);
        profile_point.right = track.right * (0.5 + relative_motion) * static_cast<double>(dir);
        profile_point.right += prev_track.right * (0.5 - relative_motion) * static_cast<double>(dir);
        profile_point.up = track.up * (0.5 + relative_motion);
        profile_point.up += prev_track.up * (0.5 - relative_motion);
        return profile_point;
    }

    if ((relative_motion > 0.5) && (idx < profile_data.size() - 1))
    {
        zds_track_t next_track = profile_data.at(idx + 1);
        profile_point.orth = track.orth * (1.5 - relative_motion) * static_cast<double>(dir);
        profile_point.orth += next_track.orth * (relative_motion - 0.5) * static_cast<double>(dir);
        profile_point.right = track.right * (1.5 - relative_motion) * static_cast<double>(dir);
        profile_point.right += next_track.right * (relative_motion - 0.5) * static_cast<double>(dir);
        profile_point.up = track.up * (1.5 - relative_motion);
        profile_point.up += next_track.up * (relative_motion - 0.5);
        return profile_point;
    }

    profile_point.orth = track.orth * static_cast<double>(dir);
    profile_point.right = track.right * static_cast<double>(dir);
    profile_point.up = track.up;
    return profile_point;
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
std::string getLine(std::istream &stream)
{
    std::string line = "";
    std::getline(stream, line);
    std::string tmp = delete_symbol(line, '\r');
    tmp = delete_symbol(tmp, ';');
    std::replace(tmp.begin(), tmp.end(), ',', ' ');

    return tmp;
}
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Profile::load(const std::string &path)
{
    if (path.empty())
    {
        Journal::instance()->error("Profile path is empty");
        return false;
    }

    std::ifstream stream(path.c_str(), std::ios::in);

    if (!stream.is_open())
    {
        std::cout << "File " << path << " not opened" << std::endl;
        Journal::instance()->error("File " + QString(path.c_str()) + " is't found");
        return false;
    }

    return load(stream);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Profile::load(std::ifstream &stream)
{
    std::vector<zds_track_t> tmp_data;

    while (!stream.eof())
    {
        std::string line = "";
        line = getLine(stream);
        if (line.empty())
            continue;

        std::istringstream ss(line);

        zds_track_t track = zds_track_t();

        ss  >> track.begin_point.x
            >> track.begin_point.y
            >> track.begin_point.z
            >> track.end_point.x
            >> track.end_point.y
            >> track.end_point.z
            >> track.prev_uid
            >> track.next_uid
            >> track.arrows
            >> track.voltage
            >> track.ordinate;

        tmp_data.push_back(track);
    }

    double route_length = 0.0;
    auto it = tmp_data.begin();
    bool not_end = true;
    while ( not_end )
    {
        zds_track_t cur_track = *it;
        if (cur_track.next_uid != -2)
        {
            zds_track_t next_track = tmp_data.at(static_cast<size_t>(cur_track.next_uid - 1));
            cur_track.end_point = next_track.begin_point;
            *it = next_track;
        }
        else
        {
            not_end = false;
        }

        dvec3 dir_vector = cur_track.end_point - cur_track.begin_point;
        cur_track.length = length(dir_vector);
        cur_track.orth = dir_vector / cur_track.length;
        cur_track.right = normalize(cross(cur_track.orth, dvec3(0.0, 0.0, 1.0)));
        cur_track.up = normalize(cross(cur_track.right, cur_track.orth));
/*
        //double yaw = (cur_track.orth.x > 0.0) ? acos(cur_track.orth.y) : Physics::PI * 2.0 - acos(cur_track.orth.y);
        double yaw = (cur_track.orth.x > 0.0) ? acos(cur_track.orth.y) : - acos(cur_track.orth.y);
        double pitch = asin(cur_track.orth.z);

        cur_track.attitude = dvec3(pitch, 0.0, yaw);
*/
        cur_track.railway_coord = route_length;
        route_length += cur_track.length;
        cur_track.inclination = cur_track.orth.z * 1000.0;
        cur_track.curvature = 0.0;

        profile_data.push_back(cur_track);
    }

    return true;
}
