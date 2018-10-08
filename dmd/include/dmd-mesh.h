//------------------------------------------------------------------------------
//
//
//
//
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief
 * \copyright
 * \author
 * \date
 */

#ifndef     DMD_MESH_H
#define     DMD_MESH_H

#include    <vector>
#include    <osg/Geometry>

/*!
 * \struct
 * \brief Face description structure
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct face_t
{
    std::vector<int>    indices;
    osg::Vec3           normal;
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct dmd_mesh_t
{
    // Raw data from file
    std::vector<osg::Vec3f>             vertices;
    std::vector<face_t>                 faces;

    // Vertices and faces data
    osg::ref_ptr<osg::Vec3Array>        vertices_array;
    osg::ref_ptr<osg::Vec3Array>        smooth_normals;
    size_t                              vertex_count;
    size_t                              faces_count;

    // Textures data
    osg::ref_ptr<osg::Vec2Array>        texture_vertices;
    std::vector<face_t>                 texture_faces;
    size_t                              tex_v_count;
    size_t                              tex_f_count;
    bool                                texture_present;

    dmd_mesh_t()
        : vertices({})
        , faces({})
        , vertices_array(nullptr)
        , smooth_normals(nullptr)
        , vertex_count(0)
        , faces_count(0)
        , texture_vertices(nullptr)
        , texture_faces({})
        , tex_v_count(0)
        , tex_f_count(0)
        , texture_present(true)
    {

    }
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct dmd_multymesh_t
{
    std::vector<dmd_mesh_t>         meshes;


    dmd_multymesh_t()
        : meshes({})        
    {

    }
};

#endif // DMD_MESH_H
