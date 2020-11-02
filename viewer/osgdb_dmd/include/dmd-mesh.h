#ifndef     DMD_MESH_H
#define     DMD_MESH_H

#include    <osg/Array>
#include    <osg/Drawable>

typedef  std::vector<unsigned int> face_t;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct dmd_mesh_t
{
    osg::ref_ptr<osg::Vec3Array>                        vertices;
    osg::ref_ptr<osg::Vec3Array>                        normals;    
    std::vector<face_t>                                 faces;
    osg::ref_ptr<osg::Vec2Array>                        texcoords;
    std::vector<face_t>                                 texfaces;
    bool                                                is_texture_present;

    osg::Vec3 calcFaceNormal(osg::DrawElementsUInt *face) const
    {
        osg::Vec3 v0 = vertices->at(face->at(0));
        osg::Vec3 v1 = vertices->at(face->at(1));
        osg::Vec3 v2 = vertices->at(face->at(2));

        osg::Vec3 n = (v1 - v0) ^ (v2 - v0);

        return n * (1 / n.length());
    }
};

#endif // DMD_MESH_H
