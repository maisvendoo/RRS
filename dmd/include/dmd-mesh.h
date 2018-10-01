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
    osg::Vec3Array      *vertices;
    osg::Vec3Array      *normals;
    std::vector<face_t> faces;
    osg::Vec2Array      *texture_vertices;
    std::vector<int>    texture_indices;
    size_t              vertex_count;
    size_t              faces_count;

    float               tx_max;
    float               ty_max;

    dmd_mesh_t()
        : vertices(nullptr)
        , normals(nullptr)
        , faces({})
        , texture_vertices(nullptr)
        , texture_indices({})
        , vertex_count(0)
        , faces_count(0)
        , tx_max(1.0f)
        , ty_max(1.0f)
    {

    }
};

#endif // DMD_MESH_H
