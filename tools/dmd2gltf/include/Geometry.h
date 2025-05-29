#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <cstdint>
#include <vector>
#include <string>

struct Vec2
{
    float x;
    float y;
};

struct Vec3
{
    float x;
    float y;
    float z;
};

struct Vertex
{
    Vec3 pos;
    Vec2 tex_coord;
};

struct Geometry
{
    std::string model_file_name = "";
    bool is_reversed_texture_coord = false;
    bool is_blend_material = false;
    std::vector<Vertex> vertices;
    std::vector<std::uint32_t> indices;
};

#endif // GEOMETRY_H
