#include    "zds-route-loader.h"

#include    <osgDB/ReadFile>
#include    <osg/Image>
#include    <osg/Texture2D>
#include    <osg/TexMat>
#include    <osg/Material>
#include    <osg/Geometry>
#include    <osg/MatrixTransform>

#include    <QDir>
#include    <QString>
#include    <QStringList>
#include    <QTextStream>
#include    <QDebug>

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
osg::Group *ZdsRouteLoader::load(QString route_path)
{
    routeRootPath = fs->combinePath(fs->getRoutesDirectory(), route_path);

    //osg::Node *model = loadModel("models/park_pass.dmd", "textures/park.tga");

    loadTracks(routeRootPath);
    loadObjectRefs(fs->combinePath(routeRootPath, "objects.ref"));
    loadRoute1Map(fs->combinePath(routeRootPath, "route1.map"));

    osg::Group *group = new osg::Group;
    loadRoute1MapNodes(group);
    //group->addChild(model);

    return group;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
osg::Vec3f ZdsRouteLoader::getPosition(float rail_coord)
{
    track_t track;
    return getPosition(rail_coord, track);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
osg::Node *ZdsRouteLoader::loadModel(QString model_path, QString texture_path)
{
    osg::Node *modelNode = nullptr;

    // Loading of model geometry
    try
    {
        std::string m_path = fs->combinePath(routeRootPath, model_path).toStdString();
        modelNode = osgDB::readNodeFile(m_path);
    }
    catch (std::exception &e)
    {
        qDebug() << e.what();
        return nullptr;
    }

    if (modelNode != nullptr)
    {
        osg::ref_ptr<osg::StateSet>   stateset = new osg::StateSet;
        osg::ref_ptr<osg::Material>   material = new osg::Material;
        stateset->setAttribute(material.get());

        material->setAmbient(osg::Material::FRONT_AND_BACK, osg::Vec4f(1.0f, 1.0f, 1.0f, 1.0f));
        material->setDiffuse(osg::Material::FRONT_AND_BACK, osg::Vec4f(1.0f, 1.0f, 1.0f, 1.0f));

        loadTexture(texture_path, stateset.get(), 0);

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
            osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D(image.get());
            osg::Texture::WrapMode textureWrapMode = osg::Texture::REPEAT;

            texture->setWrap(osg::Texture2D::WRAP_R, textureWrapMode);
            texture->setWrap(osg::Texture2D::WRAP_S, textureWrapMode);
            texture->setWrap(osg::Texture2D::WRAP_T, textureWrapMode);
            stateset->setTextureAttributeAndModes(texture_unit, texture,
                                                  osg::StateAttribute::ON);


            osg::Matrix mat;
            osg::ref_ptr<osg::TexMat> texmat = new osg::TexMat;
            texmat->setMatrix(mat);
            stateset->setTextureAttributeAndModes(texture_unit, texmat,
                                                  osg::StateAttribute::ON);

            texture->setUnRefImageDataAfterApply(true);
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
void ZdsRouteLoader::loadObjectRefs(QString path)
{
    QFile file(path);

    size_t lines_count = 0;

    if (file.open(QIODevice::ReadOnly))
    {
        QTextStream stream(&file);

        QString mode = "";

        while (!stream.atEnd())
        {
            QString line = stream.readLine();

            qDebug() << ++lines_count << ": " << line;

            if (line.isEmpty())
                continue;

            if (line.at(0) == ';')
                continue;

            if (line.at(0) == '[')
            {
                mode = line.remove('[').remove(']');
                continue;
            }

            QStringList tokens = line.split('\t');

            object_ref_t object;

            if (tokens.size() < 3)
                continue;

            object.name = tokens.at(0);
            object.mode = mode;
            object.model_path = tokens.at(1);
            object.texture_path = tokens.at(2);

            object.model_path = QDir::toNativeSeparators(object.model_path.remove(0, 1));
            object.texture_path = QDir::toNativeSeparators(object.texture_path.remove(0, 1));

            objects_refs.insert(object.name, object);
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ZdsRouteLoader::loadObjectsDat(QString path)
{
    QFile file(path);

    size_t lines_count = 0;

    if (file.open(QIODevice::ReadOnly))
    {
        QTextStream stream(&file);

        QChar   delimiter('\t');

        while (!stream.atEnd())
        {
            QString line = stream.readLine();

            if (line.isEmpty())
                continue;

            qDebug() << ++lines_count << ": " << line;

            // Ignore comments
            if (line.at(0) == ';')
                continue;

            line.truncate(line.lastIndexOf(delimiter));
            QStringList tokens = line.split(delimiter);

            if (tokens.size() < 6)
                continue;

            objects_dat_t object_dat;

            object_dat.ordinate = tokens.at(0).toFloat();
            object_dat.shift = tokens.at(1).toFloat();
            object_dat.angle_horizontal = tokens.at(2).toFloat() * M_PI / 180.0f;
            object_dat.angle_vertical = tokens.at(3).toFloat() * M_PI / 180.0f;
            object_dat.height = tokens.at(4).toFloat();
            object_dat.name = tokens.at(5);

            objects_dat.append(object_dat);
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ZdsRouteLoader::loadRoute1Map(QString path)
{
    QFile   file(path);

    if (file.open(QIODevice::ReadOnly))
    {
        QTextStream stream(&file);
        QChar   delimiter(',');

        size_t lines_count = 0;

        while (!stream.atEnd())
        {
            QString line = stream.readLine();

            if (line.at(0) == ';')
                continue;

            line.truncate(line.lastIndexOf(';'));
            QStringList tokens = line.split(delimiter);

            object_map_t object_map;

            object_map.name = tokens.at(0);
            object_map.x = tokens.at(1).toFloat();
            object_map.y = tokens.at(2).toFloat();
            object_map.z = tokens.at(3).toFloat();
            object_map.angle_x = tokens.at(4).toFloat() * M_PI / 180.0f;
            object_map.angle_y = tokens.at(5).toFloat() * M_PI / 180.0f;
            object_map.angle_z = tokens.at(6).toFloat() * M_PI / 180.0f;

            object_map.caption = stream.readLine();

            objects_map.append(object_map);
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ZdsRouteLoader::loadObjectsDatNodes(osg::Group *root_node)
{
    size_t nodes_count = 0;

    foreach (objects_dat_t object_dat, objects_dat)
    {
        node_t node;

        auto it = nodes.find(object_dat.name);

        if (it.key() != object_dat.name)
        {
            auto it = objects_refs.find(object_dat.name);

            if (it.key() != object_dat.name)
                continue;

            object_ref_t object_ref = it.value();
            node.name = object_dat.name;

            qDebug() << "Loading " << object_dat.name << "...";

            node.node = loadModel(object_ref.model_path, object_ref.texture_path);

            if (node.node == nullptr)
                continue;

            nodes.insert(node.name, node);

            qDebug() << "OK: Loaded " << object_dat.name << " successfuly... count: "
                     << ++nodes_count;

        }
        else
            node = it.value();

        track_t track;
        osg::Vec3f track_pos = getPosition(object_dat.ordinate, track);
        track_pos += track.right *= object_dat.shift;
        track_pos += osg::Vec3f(0, 0, 1.0f) *= object_dat.height;

        osg::MatrixTransform *transform1 = new osg::MatrixTransform;


        osg::Matrix m1 = osg::Matrix::translate(track_pos);
        osg::Matrix m2 = osg::Matrix::rotate(object_dat.angle_horizontal, osg::Vec3f(0, 0, 1));
        osg::Matrix m3 = osg::Matrix::rotate(object_dat.angle_vertical, osg::Vec3f(1, 0, 0));

        osg::Matrix m = m1 * m2 * m3;

        transform1->setMatrix(m);

        transform1->addChild(node.node);
        root_node->addChild(transform1);

        if (nodes_count == 50)
            break;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ZdsRouteLoader::loadRoute1MapNodes(osg::Group *group)
{
    size_t nodes_count = 0;

    osg::Vec3f pos = getPosition(20.0f);

    foreach (object_map_t object_map, objects_map)
    {
        if (object_map.name.isEmpty())
            continue;

        osg::Vec3f obj_pos(object_map.x, object_map.y, object_map.z);

        osg::Vec3f dist = obj_pos - pos;

        if (dist.length() > 20000.0f)
            continue;

        node_t node;

        auto it =nodes.find(object_map.name);

        if (it.key() != object_map.name)
        {
            auto it = objects_refs.find(object_map.name);

            if (it.key() != object_map.name)
                continue;

            object_ref_t object_ref = it.value();
            node.name = object_map.name;

            qDebug() << "Loading " << object_map.name << "...";

            node.node = loadModel(object_ref.model_path, object_ref.texture_path);

            if (node.node == nullptr)
                continue;

            nodes.insert(node.name, node);

            qDebug() << "OK: Loaded " << object_map.name << " successfuly... count: "
                     << ++nodes_count;
        }
        else
            node = it.value();

        osg::MatrixTransform *transform1 = new osg::MatrixTransform;

        osg::Matrix m1 = osg::Matrix::translate(obj_pos);
        osg::Matrix m2 = osg::Matrix::rotate(-object_map.angle_z, osg::Vec3f(0, 0, 1));
        osg::Matrix m3 = osg::Matrix::rotate(-object_map.angle_x, osg::Vec3f(1, 0, 0));
        osg::Matrix m4 = osg::Matrix::rotate(-object_map.angle_y, osg::Vec3f(0, 1, 0));

        osg::Matrix m = m2 * m3 * m4 * m1;

        transform1->setMatrix(m);

        transform1->addChild(node.node);
        group->addChild(transform1);

        //if (nodes_count == 100)
           // break;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ZdsRouteLoader::createSortedObjectsList()
{
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
osg::Vec3f ZdsRouteLoader::getPosition(float rail_coord, track_t &track)
{
    track = trackSearch(1, rail_coord);

    float motion = rail_coord - track.rail_coord;
    osg::Vec3f motion_vec = track.orth *= motion;
    osg::Vec3f  pos = track.begin_point + motion_vec;

    return pos;
}

GET_ROUTE_LOADER(ZdsRouteLoader)
