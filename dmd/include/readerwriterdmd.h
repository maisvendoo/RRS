#ifndef     READER_WRITER_DMD_H
#define     READER_WRITER_DMD_H

#include    <osgDB/Registry>
#include    <osgDB/FileUtils>
#include    <osgDB/FileNameUtils>
#include    <osgDB/ReadFile>
#include    <osg/StateSet>
#include    <osg/Material>

#include    <iostream>

class ReaderWriteDMD : public osgDB::ReaderWriter
{
public:

    ReaderWriteDMD();
    virtual ~ReaderWriteDMD();

    virtual const char *className() const;

protected:


};

#endif // READER_WRITER_DMD_H
