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
    /// Массив вершин
    osg::ref_ptr<osg::Vec3Array>                        vertices;
    /// Массив индексов вершин граней
    std::vector<face_t>                                 faces;
    /// Массив вершинных нормалей
    osg::ref_ptr<osg::Vec3Array>                        vertex_normals;
    /// Массив сглаженных нормалей
    osg::ref_ptr<osg::Vec3Array>                        smooth_normals;
    /// Число вершин
    unsigned int                                        vertex_count;
    /// Число ганей
    unsigned int                                        faces_count;

    osg::Vec3 calcFaceNormal(const face_t &face) const
    {
        osg::Vec3 v0 = vertices->at(face[0]);
        osg::Vec3 v1 = vertices->at(face[1]);
        osg::Vec3 v2 = vertices->at(face[2]);

        osg::Vec3 n = (v1 - v0) ^ (v2 - v0);

        return n * (1 / n.length());
    }
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct dmd_multimesh_t
{
    std::vector<dmd_mesh_t>         meshes;
    osg::ref_ptr<osg::Vec2Array>    texvrtices;
    std::vector<face_t>             texfaces;
    unsigned int                    tex_v_count;
    unsigned int                    tex_f_count;
    bool                            is_texture_present;
};

#endif // DMD_MESH_H
