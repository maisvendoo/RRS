//------------------------------------------------------------------------------
//
//
//
//
//------------------------------------------------------------------------------
#ifndef     READER-WRITER_H
#define     READER-WRITER_H

#include    <osg/Geometry>
#include    <osg/Geode>
#include    <osgDB/FileUtils>
#include    <osgDB/FileNameUtils>
#include    <osgDB/Registry>
#include    <osgUtil/SmoothingVisitor>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class ReaderWriterDMD : public osgDB::ReaderWriter
{
public:

    ReaderWriterDMD();

    virtual ReadResult readNode(const std::string &filepath,
                                const osgDB::Options *options) const;

    virtual ReadResult readNode(std::ifstream &stream,
                                const osgDB::Options *options) const;

    virtual WriteResult writeNode(const osg::Node &node, const std::string &path,
                                  const osgDB::Options *options) const;

    virtual WriteResult writeNode(const osg::Node &node, std::ostream &path,
                                  const osgDB::Options *options) const;

private:


};

#endif // READER-WRITER_H
