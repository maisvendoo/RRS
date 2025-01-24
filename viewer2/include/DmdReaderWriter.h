#ifndef DMD_READER_WRITER_H
#define DMD_READER_WRITER_H

#include <vsg/all.h>
#include <vsg/core/Inherit.h>
#include <vsg/core/Object.h>
#include <vsg/io/Options.h>
#include <vsg/io/Path.h>
#include <vsg/io/ReaderWriter.h>

class DmdReaderWriter : public vsg::Inherit<vsg::ReaderWriter, DmdReaderWriter>
{
public:
    DmdReaderWriter();
    ~DmdReaderWriter();

    vsg::ref_ptr<vsg::Object> read(const vsg::Path& filename, vsg::ref_ptr<const vsg::Options> options = {}) const override;
};

#endif // DMD_READER_WRITER_H
