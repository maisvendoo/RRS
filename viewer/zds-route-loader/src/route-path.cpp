//------------------------------------------------------------------------------
//
//      Tracks loader for ZDSimulator routes
//      (c) maisvendoo, 26/11/2018
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief Tracks loader for ZDSimulator routes
 * \copyright maisvendoo
 * \author maisvendoo
 * \date 26/11/2018
 */

#include    "route-path.h"
#include    "string-funcs.h"

#include    <sstream>
#include    <vector>

#include    <osg/PagedLOD>
#include    <osg/Geode>
#include    <osg/Material>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
RoutePath::RoutePath(const std::string &track_file_path)
    : length(0.0f)
{
    load(track_file_path);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
osg::Vec3 RoutePath::getPosition(float railway_coord)
{
    osg::Vec3 attitude;
    return getPosition(railway_coord, attitude);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
osg::Vec3 RoutePath::getPosition(float railway_coord, osg::Vec3 &attitude)
{
    track_t next_track;
    track_t track = findTrack(railway_coord, next_track);

    float motion = railway_coord - track.rail_coord;
    osg::Vec3 motion_vec = track.orth *= motion;
    osg::Vec3 pos = track.begin_point + motion_vec;

    float t = motion / track.length;
    attitude = track.attitude *= (1.0f - t);
    attitude += next_track.attitude * t;

    return pos;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float RoutePath::getLength() const
{
    return length;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
osg::Group *RoutePath::getTrackLine(const osg::Vec4 &color)
{
    if (track_data.size() == 0)
        return nullptr;

    osg::ref_ptr<osg::Group> trackLine = new osg::Group;


    for (auto it = track_data.begin(); it != track_data.end(); ++it)
    {
        track_t track = *it;
        osg::ref_ptr<osg::PagedLOD> line = new osg::PagedLOD;
        line->setRange(0, 0.0f, FLT_MAX);

        osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
        osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
        osg::Vec3 h(0.0f, 0.0f, 1.0f);
        vertices->push_back(track.begin_point + h);
        vertices->push_back(track.end_point + h);
        geom->setVertexArray(vertices.get());

        osg::ref_ptr<osg::DrawArrays> da = new osg::DrawArrays(osg::PrimitiveSet::LINES,
                                                               0,
                                                               static_cast<int>(vertices->size()));
        geom->addPrimitiveSet(da.get());

        osg::ref_ptr<osg::Geode> geode = new osg::Geode;
        geode->addDrawable(geom.get());
        geode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

        osg::ref_ptr<osg::Material> material = new osg::Material;
        material->setAmbient(osg::Material::FRONT_AND_BACK, color);
        material->setDiffuse(osg::Material::FRONT_AND_BACK, color);
        material->setEmission(osg::Material::FRONT_AND_BACK, color);
        geode->getOrCreateStateSet()->setAttribute(material.get());

        line->addChild(geode.get());
        trackLine->addChild(line.get());
    }

    return trackLine.release();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool RoutePath::load(const std::string &track_file_path)
{
    std::string ext = osgDB::getLowerCaseFileExtension(track_file_path);

    if (ext != "trk")
        return false;

    std::string fileName = osgDB::findDataFile(track_file_path, osgDB::CASE_INSENSITIVE);

    if (fileName.empty())
        return false;

    std::ifstream stream(fileName.c_str(), std::ios::in);

    if (!stream)
        return false;

    return load(stream);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float arg(float cos_x, float sin_x)
{
    float angle = 0;

    if (sin_x >= 0.0f)
        angle = acosf(cos_x);
    else
        angle = -acosf(cos_x);

    return angle;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool RoutePath::load(std::istream &stream)
{
    track_data_t tmp_data;

    while (!stream.eof())
    {
        std::string line = getLine(stream);

        std::istringstream ss(line);

        track_t track;

        ss >> track.begin_point.x()
           >> track.begin_point.y()
           >> track.begin_point.z()
           >> track.end_point.x()
           >> track.end_point.y()
           >> track.end_point.z()
           >> track.prev_uid
           >> track.next_uid
           >> track.arrows
           >> track.voltage
           >> track.ordinate;

        osg::Vec3 dir_vector = track.end_point - track.begin_point;
        track.length = dir_vector.length();
        track.orth = dir_vector *= (1 / track.length);

        float yaw = arg(track.orth.y(), track.orth.x());
        float pitch = asinf(track.orth.z());

        track.attitude = osg::Vec3(pitch, 0.0f, yaw);

        osg::Vec3 up(0, 0, 1);
        track.right = track.orth ^ up;
        track.right = track.right *= (1 / track.right.length());        

        tmp_data.push_back(track);
    }

    track_data.push_back(*tmp_data.begin());
    length += (*tmp_data.begin()).length;

    float rail_coord = 0.0f;

    for (auto it = tmp_data.begin(); (*it).next_uid != -2; ++it)
    {
        track_t cur_track = *it;
        track_t next_track = tmp_data.at(static_cast<size_t>(cur_track.next_uid - 1));
        track_data[track_data.size() - 1].end_point = next_track.begin_point;
        length += next_track.length;

        rail_coord += track_data.at(track_data.size() - 1).length;
        next_track.rail_coord = rail_coord;

        track_data.push_back(next_track);
    }

    return false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::string RoutePath::getLine(std::istream &stream) const
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
track_t RoutePath::findTrack(float railway_coord, track_t &next_track)
{
    track_t track;

    size_t left_idx = 0;
    size_t right_idx = track_data.size() - 1;
    size_t idx = (left_idx + right_idx) / 2;

    while (idx != left_idx)
    {
        track = track_data.at(idx);

        if (railway_coord <= track.rail_coord)
            right_idx = idx;
        else
            left_idx = idx;

        idx = (left_idx + right_idx) / 2;
    }

    if (idx < right_idx)
        next_track = track_data.at(idx + 1);
    else
        next_track = track_data.at(idx);

    return track_data.at(idx);
}


