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

    dmd_multimesh_t multimesh = load_dmd(stream);

    if (multimesh.meshes.size() > 1)
    {
        int zu = 0;
    }

    osg::ref_ptr<osg::Geode> geode = new osg::Geode;

    osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
    osg::ref_ptr<osg::Vec3Array> normals = new osg::Vec3Array;
    osg::ref_ptr<osg::Vec2Array> texcoords = new osg::Vec2Array;

    dmd_mesh_t mesh = multimesh.meshes[0];

    for (unsigned int i = 0; i < mesh.faces_count; ++i)
    {
        face_t face = mesh.faces[i];

        vertices->push_back(mesh.vertices->at(face[0]));
        vertices->push_back(mesh.vertices->at(face[1]));
        vertices->push_back(mesh.vertices->at(face[2]));
    }

    osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
    geom->setVertexArray(vertices.get());
    //geom->setNormalArray(normals.get());
    //geom->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
    geom->addPrimitiveSet(new osg::DrawArrays(GL_TRIANGLES, 0, mesh.faces_count * 3));

    osgUtil::SmoothingVisitor::smooth(*geom);

    if (multimesh.is_texture_present)
    {
        for (unsigned int i = 0; i < multimesh.tex_f_count; ++i)
        {
            face_t texface = multimesh.texfaces[i];

            texcoords->push_back(multimesh.texvrtices->at(texface[0]));
            texcoords->push_back(multimesh.texvrtices->at(texface[1]));
            texcoords->push_back(multimesh.texvrtices->at(texface[2]));
        }

        geom->setTexCoordArray(0, texcoords.get());
    }

    geode->addDrawable(geom.get());

    return geode.release();
}


REGISTER_OSGPLUGIN( dmd, ReaderDMD )
