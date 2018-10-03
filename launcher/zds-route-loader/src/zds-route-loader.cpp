#include    "zds-route-loader.h"

#include    <osgDB/ReadFile>
#include    <osg/Image>
#include    <osg/Texture2D>
#include    <osg/TexMat>
#include    <osg/Material>
#include    <osg/Geometry>

#include    <QDir>
#include    <QString>
#include    <QStringList>
#include    <QTextStream>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ZdsRouteLoader::ZdsRouteLoader(QObject *parent) : RouteLoader(parent)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ZdsRouteLoader::~ZdsRouteLoader()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
osg::Node *ZdsRouteLoader::load(QString route_path)
{
    routeRootPath = fs->combinePath(fs->getRoutesDirectory(), route_path);

    osg::Node *routeRoot = loadModel("traffic/ep20.dmd", "traffic/ep20.bmp");

    loadTracks(routeRootPath);

    loadObjectRefs(routeRootPath);

    return routeRoot;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
osg::Vec3f ZdsRouteLoader::getPosition(float rail_coord)
{
    track_t track = trackSearch(1, rail_coord);

    float motion = rail_coord - track.rail_coord;
    osg::Vec3f motion_vec = track.orth *= motion;
    osg::Vec3f  pos = track.begin_point + motion_vec;

    return pos;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
osg::Node *ZdsRouteLoader::loadModel(QString model_path, QString texture_path)
{
    osg::Node *modelNode = osgDB::readNodeFile(fs->combinePath(routeRootPath, model_path).toStdString());

    if (modelNode != nullptr)
    {
        osg::StateSet   *stateset = new osg::StateSet;
        osg::Material   *material = new osg::Material;
        stateset->setAttribute(material);

        material->setAmbient(osg::Material::FRONT_AND_BACK, osg::Vec4f(1.0f, 1.0f, 1.0f, 1.0f));
        material->setDiffuse(osg::Material::FRONT_AND_BACK, osg::Vec4f(1.0f, 1.0f, 1.0f, 1.0f));

        loadTexture(texture_path, stateset, 0);

        modelNode->setStateSet(stateset);
    }

    return modelNode;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ZdsRouteLoader::loadTexture(QString texture_path,
                                 osg::StateSet *stateset,
                                 const unsigned int texture_unit)
{
    if (!texture_path.toStdString().empty())
    {
        osg::ref_ptr<osg::Image> image;
        image = osgDB::readRefImageFile(fs->combinePath(routeRootPath, texture_path).toStdString());

        if (image.valid())
        {
            osg::Texture2D *texture = new osg::Texture2D(image.get());
            osg::Texture::WrapMode textureWrapMode = osg::Texture::REPEAT;

            texture->setWrap(osg::Texture2D::WRAP_R, textureWrapMode);
            texture->setWrap(osg::Texture2D::WRAP_S, textureWrapMode);
            texture->setWrap(osg::Texture2D::WRAP_T, textureWrapMode);
            stateset->setTextureAttributeAndModes(texture_unit, texture,
                                                  osg::StateAttribute::ON);


            osg::Matrix mat;
            osg::TexMat *texmat = new osg::TexMat;
            texmat->setMatrix(mat);
            stateset->setTextureAttributeAndModes(texture_unit, texmat,
                                                  osg::StateAttribute::ON);
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QStringList ZdsRouteLoader::findTrackFiles(QString route_dir_path)
{
    QDir    route_dir(route_dir_path);

    QStringList track_files = route_dir.entryList(QStringList("*.trk"), QDir::Files);

    return track_files;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ZdsRouteLoader::loadTracks(QString route_dir_path)
{
    QStringList track_files = findTrackFiles(route_dir_path);

    foreach (QString track_file, track_files)
    {
        QFileInfo info(track_file);
        int track_idx = info.baseName().remove("route").toInt();

        tracks_data_t tracks_data = loadTrackFile(fs->combinePath(route_dir_path, track_file));

        tracks.insert(track_idx, tracks_data);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
tracks_data_t ZdsRouteLoader::loadTrackFile(QString path)
{
    tracks_data_t tracks_data;
    QFile   file(path);

    if (file.open(QIODevice::ReadOnly))
    {
        QTextStream stream(&file);

        float rail_coord = 0.0f;

        while (!stream.atEnd())
        {
            QString line = stream.readLine().remove(';');
            QStringList tokens = line.split(',');

            track_t track;

            float x1 = tokens.at(0).toFloat();
            float y1 = tokens.at(1).toFloat();
            float z1 = tokens.at(2).toFloat();

            track.begin_point = osg::Vec3f(x1, y1, z1);

            float x2 = tokens.at(3).toFloat();
            float y2 = tokens.at(4).toFloat();
            float z2 = tokens.at(5).toFloat();

            track.end_point = osg::Vec3f(x2, y2, z2);

            track.prev_uid = tokens.at(6).toInt();
            track.next_uid = tokens.at(7).toInt();
            track.arrows = tokens.at(8);
            track.voltage = tokens.at(9).toInt();
            track.ordinate = tokens.at(10).toInt();

            osg::Vec3f dir_vector = track.end_point - track.begin_point;
            track.length = dir_vector.length();
            track.orth = dir_vector *= (1 / track.length);

            osg::Vec3f up(0, 0, 1);
            track.right = track.orth ^ up;
            track.right = track.right *= (1 / track.right.length());

            track.rail_coord = rail_coord;
            rail_coord += track.length;

            tracks_data.append(track);
        }
    }

    return tracks_data;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
track_t ZdsRouteLoader::trackSearch(int track_idx, float rail_coord)
{
    track_t track;

    tracks_data_t tracks_data = tracks[track_idx];

    int left_idx = 0;
    int right_idx = tracks_data.size() - 1;
    int idx = (left_idx + right_idx) / 2;

    while (idx != left_idx)
    {
        track = tracks_data.at(idx);

        if (rail_coord < track.rail_coord)
            right_idx = idx;
        else
            left_idx = idx;

        idx = (left_idx + right_idx) / 2;
    }

    return track;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ZdsRouteLoader::loadObjectRefs(QString route_dir)
{
    QFile file(fs->combinePath(route_dir, "objects.ref"));

    if (file.open(QIODevice::ReadOnly))
    {
        QTextStream stream(&file);

        QString mode = "";

        while (!stream.atEnd())
        {
            QString line = stream.readLine();

            if (line.at(0) == ';')
                continue;

            if (line.at(0) == '[')
            {
                mode = line.remove('[').remove(']');
                continue;
            }

            QStringList tokens = line.split('\t');

            object_t object;

            object.name = tokens.at(0);
            object.mode = mode;
            object.model_path = tokens.at(1);
            object.texture_path = tokens.at(2);

            object.model_path = QDir::toNativeSeparators(object.model_path.remove(0, 1));
            object.texture_path = QDir::toNativeSeparators(object.texture_path.remove(0, 1));

            objects.insert(object.name, object);
        }
    }
}

GET_ROUTE_LOADER(ZdsRouteLoader)
