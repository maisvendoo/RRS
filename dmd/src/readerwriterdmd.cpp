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

    // Here do loading DMD-model!!!

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


REGISTER_OSGPLUGIN(dmd, ReaderWriterDMD)
