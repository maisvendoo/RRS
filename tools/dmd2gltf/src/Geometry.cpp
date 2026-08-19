#include    <Geometry.h>
#include    <Logger.h>

#include    <cmath>
#include    <cstdint>
#include    <filesystem>
#include    <fstream>
#include    <map>
#include    <string>
#include    <vector>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Geometry::get_dmd_model_data(std::string &in_dmd_model_path, bool smooth)
{
    using PosIndex = std::uint32_t;
    using TexIndex = std::uint32_t;
    using VertexIndex = std::uint32_t;

    std::ifstream model_file(in_dmd_model_path, std::ios::in);
    if (!model_file.is_open())
    {
        LOG_WARN("Warn: failed to open model file: %s", in_dmd_model_path.c_str());
        return false;
    }

    std::string file_name = std::filesystem::path(in_dmd_model_path).filename().string();
    model_file_name = file_name.substr(0, file_name.find_last_of('.'));

    std::string buffer;
    while (buffer != "TriMesh()")
    {
        model_file >> buffer;
        if (!model_file)
        {
            LOG_WARN("Warn: failed to find \"TriMesh()\" in file: %s", in_dmd_model_path.c_str());
            return false;
        }
    }

    model_file >> buffer >> buffer;

    std::uint32_t pos_count, pos_face_count;
    model_file >> pos_count >> pos_face_count;

    model_file >> buffer >> buffer;

    std::vector<Vec3> positions(pos_count);
    for (auto& pos : positions)
    {
        model_file >> pos.x >> pos.y >> pos.z;
    }

    if (!model_file)
    {
        LOG_WARN("Warn: failed to read positions from file: %s", in_dmd_model_path.c_str());
    }

    model_file >> buffer >> buffer >> buffer >> buffer;

    std::vector<PosIndex> pos_indices(pos_face_count * 3);
    for (auto& index : pos_indices)
    {
        model_file >> index;
        --index;
    }

    if (!model_file)
    {
        LOG_WARN("Warn: failed to read position indices from file: %s", in_dmd_model_path.c_str());
        return false;
    }

    while (buffer != "Texture:")
    {
        model_file >> buffer;
        if (!model_file)
        {
            LOG_WARN("Warn: failed to find \"Texture:\" in file: %s", in_dmd_model_path.c_str());
            return false;
        }
    }

    model_file >> buffer >> buffer;

    std::uint32_t tex_coord_count, tex_face_count;
    model_file >> tex_coord_count >> tex_face_count;

    if (pos_face_count != tex_face_count)
    {
        LOG_WARN("Warn: position face count is not equal to texture face count in file: %s", in_dmd_model_path.c_str());
        return false;
    }

    std::uint32_t face_count = pos_face_count;

    model_file >> buffer >> buffer;

    std::vector<Vec2> tex_coords(tex_coord_count);
    for (auto& tex_coord : tex_coords)
    {
        model_file >> tex_coord.x >> tex_coord.y >> buffer;

        if (is_reversed_texture_coord)
        {
            tex_coord.y = 1.0f - tex_coord.y;
        }
    }

    if (!model_file)
    {
        LOG_WARN("Warn: failed to read texture coordinates from file: %s", in_dmd_model_path.c_str());
        return false;
    }

    model_file >> buffer >> buffer >> buffer >> buffer >> buffer;

    std::vector<TexIndex> tex_indices(face_count * 3);
    for (auto& index : tex_indices)
    {
        model_file >> index;
        --index;
    }

    if (!model_file)
    {
        LOG_WARN("Warn: failed to read texture indices from file: %s", in_dmd_model_path.c_str());
        return false;
    }

    model_file.close();


    using Tile_index_t = int;
    using Vertex_tile_t = std::tuple<int, int, int>;
    using Face_unique_t = std::tuple<Vertex_tile_t, Vertex_tile_t, Vertex_tile_t>;
    struct Face_t
    {
        Vec3 vertex[3];
        Vec2 t_coord[3];
        Vec3 normal;
        float normal_length2;
        int8_t normal_variant[3] = {0, 0, 0};
        int8_t t_coord_variant[3] = {0, 0, 0};
    };
    std::map<Face_unique_t, Face_t> unique_faces;

    using Vertex_tcoord_variants_t = std::vector<std::pair<Vec2, uint32_t>>;
    using Vertex_normal_variants_t = std::vector<std::pair<Vec3, Vertex_tcoord_variants_t>>;
    std::map<Vertex_tile_t, std::pair<Vec3, Vertex_normal_variants_t>> unique_vertices;

    auto tile = [](const float& coord) -> Tile_index_t
    {
        return static_cast<Tile_index_t>(std::round(coord * 5000.0f));
    };
    auto tile_pos = [&](const Vec3& pos) -> Vertex_tile_t
    {
        return {tile(pos.z), tile(pos.y), tile(pos.x)};
    };

    uint32_t wrong_normals_count = 0;
    uint32_t repeat_faces_count = 0;
    uint32_t reusing_vertex_count = 0;
    for (std::uint32_t i = 0; i < face_count; ++i)
    {
        const PosIndex& pos_index1 = pos_indices[i * 3];
        const PosIndex& pos_index2 = pos_indices[i * 3 + 1];
        const PosIndex& pos_index3 = pos_indices[i * 3 + 2];

        const Vec3& p1 = positions[pos_index1];
        const Vec3& p2 = positions[pos_index2];
        const Vec3& p3 = positions[pos_index3];

        const Vec3 v12 = {p2.x - p1.x, p2.y - p1.y, p2.z - p1.z};
        const Vec3 v13 = {p3.x - p1.x, p3.y - p1.y, p3.z - p1.z};
        const Vec3 n = {
            v12.y * v13.z - v12.z * v13.y,
            v12.z * v13.x - v12.x * v13.z,
            v12.x * v13.y - v12.y * v13.x
        };
        const float n_length2 = n.x * n.x + n.y * n.y + n.z * n.z;
        if (n_length2 < 1e-10)
        {
            //LOG_WARN("Wrong normal (%e) for face %u in file: %s", length, i, in_dmd_model_path.c_str());
            ++wrong_normals_count;
            continue;
        }

        const Vertex_tile_t vt1 = tile_pos(p1);
        const Vertex_tile_t vt2 = tile_pos(p2);
        const Vertex_tile_t vt3 = tile_pos(p3);
        Face_unique_t ft;
        if (vt1 < vt2)
        {
            if (vt1 < vt3)
            {
                ft = {vt1, vt2, vt3};
            }
            else
            {
                ft = {vt3, vt1, vt2};
            }
        }
        else
        {
            if (vt2 < vt3)
            {
                ft = {vt2, vt3, vt1};
            }
            else
            {
                ft = {vt3, vt1, vt2};
            }
        }

        if (unique_faces.find(ft) == unique_faces.end())
        {
            const TexIndex& tex_index1 = tex_indices[i * 3];
            const TexIndex& tex_index2 = tex_indices[i * 3 + 1];
            const TexIndex& tex_index3 = tex_indices[i * 3 + 2];

            const Vec2& t_coord1 = tex_coords[tex_index1];
            const Vec2& t_coord2 = tex_coords[tex_index2];
            const Vec2& t_coord3 = tex_coords[tex_index3];

            Face_t new_face = {p1, p2, p3,
                               t_coord1, t_coord2, t_coord3,
                               n, n_length2};

            auto find_or_create_unique_vertex_variant = [](Face_t& _face,
                                                           std::map<Vertex_tile_t, std::pair<Vec3, Vertex_normal_variants_t>>& _unique_vertices,
                                                           const Vertex_tile_t& _vt, const uint8_t _v_idx, const bool smooth,
                                                           uint32_t& _reusing_vertex_count)
            {
                const Vec3& _p = _face.vertex[_v_idx];
                const Vec3& _n = _face.normal;
                const Vec2& _t_coord = _face.t_coord[_v_idx];

                auto it = _unique_vertices.find(_vt);
                if (it == _unique_vertices.end())
                {
                    _face.normal_variant[_v_idx] = 0;
                    _face.t_coord_variant[_v_idx] = 0;
                    _unique_vertices.insert({ _vt, {_p, {{ _n, {{_t_coord, 0}} }} } });
                }
                else
                {
                    Vertex_normal_variants_t& vnv = it->second.second;
                    bool add_new_normal_variant = true;
                    for (uint8_t n_var = 0; n_var < vnv.size(); ++n_var)
                    {
                        Vec3& nv = vnv[n_var].first;
                        const float nv_length2 = nv.x * nv.x + nv.y * nv.y + nv.z * nv.z;
                        const float n_nv_dot = _n.x * nv.x + _n.y * nv.y + _n.z * nv.z;
                        const float cos_n_nv = n_nv_dot / std::sqrt(_face.normal_length2 * nv_length2);
                        const float cos_angle_between_unique_normal_vectors = smooth ? 0.7f : 0.9999f;

                        if (cos_n_nv >= cos_angle_between_unique_normal_vectors)
                        {
                            _face.normal_variant[_v_idx] = n_var;
                            add_new_normal_variant = false;

                            nv.x += _n.x;
                            nv.y += _n.y;
                            nv.z += _n.z;

                            Vertex_tcoord_variants_t& vtcv = vnv[n_var].second;
                            bool add_new_tcoord_variant = true;
                            for (uint8_t tc_var = 0; tc_var < vtcv.size(); ++tc_var)
                            {
                                if (   (std::abs(_t_coord.x - vtcv[tc_var].first.x) < 1e-5f)
                                    && (std::abs(_t_coord.y - vtcv[tc_var].first.y) < 1e-5f))
                                {
                                    _face.t_coord_variant[_v_idx] = tc_var;
                                    add_new_tcoord_variant = false;
                                    ++_reusing_vertex_count;
                                    break;
                                }
                            }
                            if (add_new_tcoord_variant)
                            {
                                _face.t_coord_variant[_v_idx] = vtcv.size();
                                vtcv.push_back({_t_coord, 0});
                            }

                            break;
                        }
                    }
                    if (add_new_normal_variant)
                    {
                        _face.normal_variant[_v_idx] = vnv.size();
                        _face.t_coord_variant[_v_idx] = 0;
                        vnv.push_back({ _n, {{_t_coord, 0}} });
                    }
                }
            };

            find_or_create_unique_vertex_variant(
                new_face, unique_vertices, vt1, 0, smooth, reusing_vertex_count);
            find_or_create_unique_vertex_variant(
                new_face, unique_vertices, vt2, 1, smooth, reusing_vertex_count);
            find_or_create_unique_vertex_variant(
                new_face, unique_vertices, vt3, 2, smooth, reusing_vertex_count);

            unique_faces.insert({ft, new_face});
        }
        else
        {
            ++repeat_faces_count;
            continue;
        }
    }
    if (wrong_normals_count)
    {
        LOG_WARN("Excluded %u faces with wrong normal in file: %s", wrong_normals_count, in_dmd_model_path.c_str());
    }
    if (repeat_faces_count)
    {
        LOG_WARN("Excluded %u repeat faces in file: %s", repeat_faces_count, in_dmd_model_path.c_str());
    }

    vertices.reserve(unique_faces.size() * 3 - reusing_vertex_count);
    for (auto& [vt, pos_and_normal_variants] : unique_vertices)
    {
        Vec3& pos = pos_and_normal_variants.first;
        for (auto& [nv, t_coord_variants] : pos_and_normal_variants.second)
        {
            const float nv_length_inv = 1.0f / std::sqrt(nv.x * nv.x + nv.y * nv.y + nv.z * nv.z);
            nv = {nv.x * nv_length_inv, nv.y * nv_length_inv, nv.z * nv_length_inv};

            for (auto& [t_coord, index] : t_coord_variants)
            {
                index = vertices.size();
                vertices.emplace_back(Vertex{pos, nv, t_coord});
            }
        }
    }

    const bool use_indices16 = (vertices.size() < 65535);
    if (use_indices16)
    {
        indices16.reserve(unique_faces.size() * 3);
    }
    else
    {
        indices32.reserve(unique_faces.size() * 3);
    }

    auto vertex_index = [](Vertex_tile_t _vt, uint8_t _normal_variant, uint8_t _tcoord_variant,
                           std::map<Vertex_tile_t, std::pair<Vec3, Vertex_normal_variants_t>>& _unique_vertices) -> uint32_t
    {
        const auto it = _unique_vertices.find(_vt);
        const Vertex_normal_variants_t& vnv = it->second.second;
        const Vertex_tcoord_variants_t& vtcv = vnv[_normal_variant].second;
        return vtcv[_tcoord_variant].second;
    };

    for (auto& [ft, face] : unique_faces)
    {
        const Vertex_tile_t vt1 = tile_pos(face.vertex[0]);
        const Vertex_tile_t vt2 = tile_pos(face.vertex[1]);
        const Vertex_tile_t vt3 = tile_pos(face.vertex[2]);

        const uint32_t index1 = vertex_index(vt1, face.normal_variant[0], face.t_coord_variant[0], unique_vertices);
        const uint32_t index2 = vertex_index(vt2, face.normal_variant[1], face.t_coord_variant[1], unique_vertices);
        const uint32_t index3 = vertex_index(vt3, face.normal_variant[2], face.t_coord_variant[2], unique_vertices);
        if (use_indices16)
        {
            indices16.push_back(index1);
            indices16.push_back(index2);
            indices16.push_back(index3);
        }
        else
        {
            indices32.push_back(index1);
            indices32.push_back(index2);
            indices32.push_back(index3);
        }
    }

    return (vertices.size() > 0);
}
