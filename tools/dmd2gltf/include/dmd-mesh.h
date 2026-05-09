/* old dmd parser
#ifndef     DMD_MESH_H
#define     DMD_MESH_H

#include    <vec.h>
#include    <vector>

using face_t = std::vector<unsigned int>;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct dmd_mesh_t
{
    /// Массив вершин
    std::vector<Vec3>                        vertices;
    /// Массив индексов вершин граней
    std::vector<face_t>                      faces;
    /// Массив вершинных нормалей
    std::vector<Vec3>                        vertex_normals;
    /// Массив сглаженных нормалей
    std::vector<Vec3>                        smooth_normals;
    /// Число вершин
    unsigned int                             vertex_count;
    /// Число ганей
    unsigned int                             faces_count;

    Vec3 calcFaceNormal(const face_t &face) const
    {
        Vec3 v0 = vertices.at(face[0]);
        Vec3 v1 = vertices.at(face[1]);
        Vec3 v2 = vertices.at(face[2]);

        Vec3 v01 = v1 - v0;
        Vec3 v02 = v2 - v0;

        Vec3 n = v01 ^ v02;

        return n * (1 / n.length());
    }
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct dmd_multimesh_t
{
    std::vector<dmd_mesh_t>         meshes;
    std::vector<Vec2>               texvrtices;
    std::vector<face_t>             texfaces;
    unsigned int                    tex_v_count;
    unsigned int                    tex_f_count;
    bool                            is_texture_present;
};

#endif // DMD_MESH_H
*/
