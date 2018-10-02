#include    "ReaderWriterDMD.h"

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
        osg::Node *node = convertModelToSceneGraph(dmdObj, options);
        return node;
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
osg::Node *ReaderWriterDMD::convertModelToSceneGraph(DMDObject &dmdObj,
                                                     const osgDB::ReaderWriter::Options *options) const
{
    dmd_multymesh_t multyMesh = dmdObj.getMultyMesh();

    if (multyMesh.meshes.empty())
        return nullptr;

    osg::Group  *group = new osg::Group;

    for (auto it = multyMesh.meshes.begin(); it != multyMesh.meshes.end(); ++it)
    {
        osg::Geometry *geometry = convertMeshToGeometry(*it, options);

        if (multyMesh.texture_vertices->size() != 0)
            geometry->setTexCoordArray(0, multyMesh.texture_vertices);

        osg::Geode *geode = new osg::Geode;
        geode->addDrawable(geometry);

        group->addChild(geode);
    }

    return group;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
osg::Geometry *ReaderWriterDMD::convertMeshToGeometry(dmd_mesh_t *mesh,
                                                      const osgDB::ReaderWriter::Options *options) const
{
    osg::Geometry *geometry = new osg::Geometry;

    geometry->setVertexArray(mesh->vertices);

    geometry->setNormalArray(mesh->smooth_normals, osg::Array::BIND_PER_VERTEX);

    std::vector<osg::DrawElementsUInt *> meshPrimitiveSets;

    for (size_t i = 0; i < mesh->faces.size(); i++)
    {
        osg::DrawElementsUInt *primitive = new osg::DrawElementsUInt(osg::PrimitiveSet::POLYGON, 0);
        meshPrimitiveSets.push_back(primitive);

        osg::DrawElementsUInt *face = meshPrimitiveSets.back();

        for (size_t j = 0; j < mesh->faces[i].indices.size(); j++)
            face->push_back(mesh->faces[i].indices[j]);

        geometry->addPrimitiveSet(face);
    }

    return geometry;
}


REGISTER_OSGPLUGIN(dmd, ReaderWriterDMD)
