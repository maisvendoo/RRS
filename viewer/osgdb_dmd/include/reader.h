//------------------------------------------------------------------------------
//
//
//
//
//------------------------------------------------------------------------------
#ifndef     READER_H
#define     READER_H

#include    <osg/Geometry>
#include    <osg/Geode>
#include    <osgDB/FileUtils>
#include    <osgDB/FileNameUtils>
#include    <osgDB/Registry>
#include    <osgUtil/SmoothingVisitor>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class ReaderDMD : public osgDB::ReaderWriter
{
public:

    ReaderDMD();

    virtual ReadResult readNode(const std::string &filepath,
                                const osgDB::Options *options) const;

    virtual ReadResult readNode(std::ifstream &stream,
                                const osgDB::Options *options) const;

private:


};

#endif // READER_H
