#include    "reader.h"
#include    "dmd-parser.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ReaderDMD::ReaderDMD()
{
    supportsExtension("dmd", "DGLEngine model file");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
osgDB::ReaderWriter::ReadResult ReaderDMD::readNode(const std::string &filepath,
                                                    const osgDB::Options *options) const
{
    std::string ext = osgDB::getLowerCaseFileExtension(filepath);

    if (!acceptsExtension(ext))
        return ReadResult::FILE_NOT_HANDLED;

    std::string fileName = osgDB::findDataFile(filepath, options);

    if (fileName.empty())
        return ReadResult::FILE_NOT_FOUND;

    std::ifstream stream(fileName.c_str(), std::ios::in);

    if (!stream)
        return ReadResult::ERROR_IN_READING_FILE;

    return readNode(stream, options);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
osgDB::ReaderWriter::ReadResult ReaderDMD::readNode(std::ifstream &stream,
                                                    const osgDB::Options *options) const
{
    (void) options;

    dmd_mesh_t mesh = load_dmd(stream);

    osg::ref_ptr<osg::Geode> geode = new osg::Geode;

    osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
    osg::ref_ptr<osg::Vec3Array> normals = new osg::Vec3Array;
    osg::ref_ptr<osg::Vec2Array> texcoords = new osg::Vec2Array;
    osg::ref_ptr<osg::DrawElementsUInt> indices = new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES);

    for (size_t i = 0; i < mesh.faces.size(); ++i)
    {
        face_t face = mesh.faces[i];

        osg::Vec3 v0 = mesh.vertices->at(face[0]);
        osg::Vec3 v1 = mesh.vertices->at(face[1]);
        osg::Vec3 v2 = mesh.vertices->at(face[2]);

        vertices->push_back(v0);
        vertices->push_back(v1);
        vertices->push_back(v2);

        indices->push_back(vertices->size() - 3);
        indices->push_back(vertices->size() - 2);
        indices->push_back(vertices->size() - 1);

        osg::Vec3 n = (v1 - v0) ^ (v2 - v0);
        n = n * (1 / n.length());

        normals->push_back(n);
        normals->push_back(n);
        normals->push_back(n);
    }

    osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
    geom->setVertexArray(vertices.get());
    geom->addPrimitiveSet(indices.get());

    if (mesh.is_texture_present)
    {
        for (size_t i = 0; i < mesh.texfaces.size(); ++i)
        {
            face_t texface = mesh.texfaces[i];
            texcoords->push_back(mesh.texcoords->at(texface[0]));
            texcoords->push_back(mesh.texcoords->at(texface[1]));
            texcoords->push_back(mesh.texcoords->at(texface[2]));
        }

        geom->setTexCoordArray(0, texcoords.get());
    }

    geom->setNormalArray(normals.get());
    geom->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);

    //osgUtil::SmoothingVisitor::smooth(*geom);

    geode->addDrawable(geom.get());

    return geode.release();
}


REGISTER_OSGPLUGIN( dmd, ReaderDMD )
