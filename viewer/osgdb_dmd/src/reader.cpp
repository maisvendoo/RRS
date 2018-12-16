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

    osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;

    if (mesh.is_texture_present)
    {
        geom->setVertexArray(mesh.texvertices.get());
        geom->setTexCoordArray(0, mesh.texcoords.get());

        for (size_t i = 0; i < mesh.texfaces.size(); ++i)
            geom->addPrimitiveSet(mesh.texfaces[i].get());
    }
    else
    {
        geom->setVertexArray(mesh.vertices.get());

        for (size_t i = 0; i < mesh.faces.size(); ++i)
            geom->addPrimitiveSet(mesh.faces[i].get());
    }

    osgUtil::SmoothingVisitor::smooth(*geom);

    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    geode->addDrawable(geom.get());

    return geode.release();
}


REGISTER_OSGPLUGIN( dmd, ReaderDMD )
