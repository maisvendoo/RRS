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
    osg::ref_ptr<osg::Vec2Array> texcoords = new osg::Vec2Array;
    osg::ref_ptr<osg::DrawElementsUInt> indices = new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES);

    for (size_t i = 0; i < mesh.faces.size(); ++i)
    {
        face_t face = mesh.faces[i];
        face_t texface = mesh.texfaces[i];

        vertices->push_back(mesh.vertices->at(face[0]));
        vertices->push_back(mesh.vertices->at(face[1]));
        vertices->push_back(mesh.vertices->at(face[2]));

        texcoords->push_back(mesh.texcoords->at(texface[0]));
        texcoords->push_back(mesh.texcoords->at(texface[1]));
        texcoords->push_back(mesh.texcoords->at(texface[2]));

        indices->push_back(vertices->size() - 3);
        indices->push_back(vertices->size() - 2);
        indices->push_back(vertices->size() - 1);
    }

    osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
    geom->setVertexArray(vertices.get());
    geom->addPrimitiveSet(indices.get());

    if (mesh.is_texture_present)
        geom->setTexCoordArray(0, texcoords.get());

    osgUtil::SmoothingVisitor::smooth(*geom);

    geode->addDrawable(geom.get());

    return geode.release();
}


REGISTER_OSGPLUGIN( dmd, ReaderDMD )
