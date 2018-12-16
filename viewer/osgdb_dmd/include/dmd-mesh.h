#ifndef     DMD_MESH_H
#define     DMD_MESH_H

#include    <osg/Array>
#include    <osg/Drawable>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct dmd_mesh_t
{
    osg::ref_ptr<osg::Vec3Array>                        vertices;
    osg::ref_ptr<osg::Vec3Array>                        texvertices;
    std::vector<osg::ref_ptr<osg::DrawElementsUInt>>    faces;
    osg::ref_ptr<osg::Vec2Array>                        texcoords;
    std::vector<osg::ref_ptr<osg::DrawElementsUInt>>    texfaces;
    bool                                                is_texture_present;
};

#endif // DMD_MESH_H
