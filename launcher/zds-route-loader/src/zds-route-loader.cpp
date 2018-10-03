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

    return routeRoot;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
osg::Node *ZdsRouteLoader::loadModel(QString model_path, QString texture_path)
{
    osg::Node *modelNode = osgDB::readNodeFile(fs->combinePath(routeRootPath, model_path).toStdString());

    osg::StateSet   *stateset = new osg::StateSet;
    osg::Material   *material = new osg::Material;
    stateset->setAttribute(material);

    material->setAmbient(osg::Material::FRONT_AND_BACK, osg::Vec4f(1.0f, 1.0f, 1.0f, 1.0f));
    material->setDiffuse(osg::Material::FRONT_AND_BACK, osg::Vec4f(1.0f, 1.0f, 1.0f, 1.0f));

    loadTexture(texture_path, stateset, 0);

    modelNode->setStateSet(stateset);

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

            tracks_data.append(track);
        }
    }

    return tracks_data;
}

GET_ROUTE_LOADER(ZdsRouteLoader)
