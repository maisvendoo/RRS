//------------------------------------------------------------------------------
//
//      Plugin for work with DGLEngine DMD-models in OSG
//      (c) maisvendoo, 01/10/2018
//      Developer: Dmitry Pritykin
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief Plugin for work with DGLEngine DMD-models in OSG
 * \copyright maisvendoo
 * \author Dmitry Pritykin
 * \date 01/10/2018
 */

#ifndef     READER_WRITER_DMD_H
#define     READER_WRITER_DMD_H

#include    <osgDB/Registry>
#include    <osgDB/FileUtils>
#include    <osgDB/FileNameUtils>
#include    <osgDB/ReadFile>
#include    <osg/StateSet>
#include    <osg/Material>

#include    <iostream>

#include    "dmd.h"

/*!
 * \class
 * \brief Reader and writer of DMD-models
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class ReaderWriterDMD : public osgDB::ReaderWriter
{
public:

    ReaderWriterDMD();

    virtual ~ReaderWriterDMD();

    virtual const char *className() const;

    virtual ReadResult readNode(const std::string &file,
                                const Options *options) const;

    virtual ReadResult readNode(std::ifstream &fin,
                                const Options *options) const;

    virtual ReadResult doReadNode(std::ifstream &fin,
                                  const Options *options,
                                  const std::string &fileName) const;

    virtual WriteResult writeNode(const osg::Node &node,
                                  const std::string &file,
                                  const Options *options = NULL) const;

    virtual WriteResult writeNode(const osg::Node &node,
                                  const std::ofstream &fout,
                                  const Options *options = NULL) const;

    virtual WriteResult doWriteNode(const osg::Node &node,
                                    const std::ofstream &fout,
                                    const Options *options,
                                    const std::string &fileName) const;


protected:

    osg::Node *convertModelToSceneGraph(DMDObject &dmdObj,
                                        const Options *options) const;

    osg::Geometry *convertMeshToGeometry(dmd_mesh_t *mesh,
                                         const Options *options) const;
};

#endif // READER_WRITER_DMD_H
