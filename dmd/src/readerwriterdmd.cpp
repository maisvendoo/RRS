#include    "ReaderWriterDMD.h"

#include    <osg/Array>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ReaderWriterDMD::ReaderWriterDMD()
{
    supportsExtension("dmd", "DGLEngine model format");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ReaderWriterDMD::~ReaderWriterDMD()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
const char *ReaderWriterDMD::className() const
{
    return "DGLEngine DMD-models Reader/Writer";
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
osgDB::ReaderWriter::ReadResult ReaderWriterDMD::readNode(const std::string &file,
                                                         const osgDB::ReaderWriter::Options *options) const
{
    std::string ext = osgDB::getLowerCaseFileExtension(file);

    if (!acceptsExtension(ext))
        return ReadResult::FILE_NOT_HANDLED;

    std::string fileName = osgDB::findDataFile(file, options);

    if (fileName.empty())
        return ReadResult::FILE_NOT_FOUND;

    osgDB::ifstream fin(fileName.c_str(), std::ios_base::in);

    if (!fin.good())
        return ReadResult::ERROR_IN_READING_FILE;

    return doReadNode(fin, options, fileName);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
osgDB::ReaderWriter::ReadResult ReaderWriterDMD::readNode(std::ifstream &fin,
                                                         const osgDB::ReaderWriter::Options *options) const
{
    return doReadNode(fin, options, "");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
osgDB::ReaderWriter::ReadResult ReaderWriterDMD::doReadNode(std::ifstream &fin,
                                                           const osgDB::ReaderWriter::Options *options,
                                                           const std::string &fileName) const
{
    (void) fileName;
    (void) options;

    DMDObject dmdObj;

    if (dmdObj.load(fin))
    {
        fin.close();
        osg::ref_ptr<osg::Node> node = convertModelToSceneGraph(dmdObj, options);
        return node.get();
    }

    return ReadResult::FILE_NOT_HANDLED;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
osgDB::ReaderWriter::WriteResult ReaderWriterDMD::writeNode(const osg::Node &node,
                                                           const std::string &file,
                                                           const osgDB::ReaderWriter::Options *options) const
{
    (void) node;
    (void) file;
    (void) options;

    return WriteResult::FILE_NOT_HANDLED;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
osgDB::ReaderWriter::WriteResult ReaderWriterDMD::writeNode(const osg::Node &node,
                                                           const std::ofstream &fout,
                                                           const osgDB::ReaderWriter::Options *options) const
{
    (void) node;
    (void) fout;
    (void) options;

    return WriteResult::FILE_NOT_HANDLED;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
osgDB::ReaderWriter::WriteResult ReaderWriterDMD::doWriteNode(const osg::Node &node,
                                                             const std::ofstream &fout,
                                                             const osgDB::ReaderWriter::Options *options,
                                                             const std::string &fileName) const
{
    (void) node;
    (void) fileName;
    (void) fout;
    (void) options;

    return WriteResult::FILE_NOT_HANDLED;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
osg::ref_ptr<osg::Node> ReaderWriterDMD::convertModelToSceneGraph(DMDObject &dmdObj,
                                                     const osgDB::ReaderWriter::Options *options) const
{
    dmd_mesh_t *mesh = dmdObj.getMesh();

    osg::Group  *group = new osg::Group;

    osg::ref_ptr<osg::Geometry> geometry = convertMeshToGeometry(*mesh, options);
    geometry->setTexCoordArray(0, mesh->texture_vertices.get());

    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    geode->addDrawable(geometry.get());

    group->addChild(geode.get());

    return group;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
osg::ref_ptr<osg::Geometry> ReaderWriterDMD::convertMeshToGeometry(dmd_mesh_t &mesh,
                                                                   const osgDB::ReaderWriter::Options *options) const
{
    osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;

    geometry->setVertexArray(mesh.vertices_array.get());
    geometry->setNormalArray(mesh.smooth_normals.get(), osg::Array::BIND_PER_VERTEX);

    std::vector<osg::DrawElementsUInt *> meshPrimitiveSets;

    for (size_t i = 0; i < mesh.texture_faces.size(); i++)
    {
        osg::ref_ptr<osg::DrawElementsUInt> primitive = new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES, 0);
        meshPrimitiveSets.push_back(primitive.get());

        osg::ref_ptr<osg::DrawElementsUInt> face = meshPrimitiveSets.back();

        for (size_t j = 0; j < mesh.texture_faces[i].indices.size(); j++)
            face->push_back(mesh.texture_faces[i].indices[j]);

        geometry->addPrimitiveSet(face.get());
    }

    return geometry;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
osg::Vec3 ReaderWriterDMD::toRightBasis(osg::Vec3 &v) const
{
    osg::Vec3 axis(1.0f, 0.0f, 0.0f);
    osg::Quat q(osg::PIf / 2.0, axis);

    osg::Vec3 tmp = q * v;

    return osg::Vec3(tmp.x(), -tmp.y(), tmp.z());
}



REGISTER_OSGPLUGIN(dmd, ReaderWriterDMD)
