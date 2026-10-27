//------------------------------------------------------------------------------
//
//      Collision detection engine for RRS (based on Jolt Physics)
//      Collider definitions config (colliders.conf)
//
//------------------------------------------------------------------------------

#include    "collision-collider-config.h"

#include    <fstream>
#include    <sstream>
#include    <vector>

namespace collision
{

namespace
{

bool parseFloat(const std::string& token, float& value)
{
    try
    {
        std::size_t used = 0;
        value = std::stof(token, &used);
        return used == token.size();
    }
    catch (...)
    {
        return false;
    }
}

bool parseVec3(const std::vector<std::string>& tokens,
               std::size_t& pos,
               Vec3f& value)
{
    if (pos + 2 >= tokens.size())
        return false;

    return parseFloat(tokens[pos++], value.x) &&
           parseFloat(tokens[pos++], value.y) &&
           parseFloat(tokens[pos++], value.z);
}

bool parseEntry(const std::vector<std::string>& tokens, ColliderConfig& out)
{
    if (tokens.size() < 2)
        return false;

    const std::string& label = tokens[0];
    const std::string& type_name = tokens[1];

    ColliderEntry entry;
    std::size_t pos = 2;

    if (type_name == "none")
    {
        entry.type = ColliderType::None;
    }
    else if (type_name == "box")
    {
        entry.type = ColliderType::Box;
        if (!parseVec3(tokens, pos, entry.half_extents))
            return false;
    }
    else if (type_name == "sphere")
    {
        entry.type = ColliderType::Sphere;
        if (pos >= tokens.size() || !parseFloat(tokens[pos++], entry.radius))
            return false;
    }
    else if (type_name == "capsule" || type_name == "cylinder")
    {
        entry.type = (type_name == "capsule") ? ColliderType::Capsule
                                              : ColliderType::Cylinder;
        if (pos + 1 >= tokens.size() ||
            !parseFloat(tokens[pos++], entry.half_height) ||
            !parseFloat(tokens[pos++], entry.radius))
            return false;
    }
    else if (type_name == "mesh")
    {
        entry.type = ColliderType::Mesh;
        if (pos >= tokens.size())
            return false;
        entry.file = tokens[pos++];
    }
    else
    {
        return false;
    }

    // Опциональные параметры в любом порядке
    while (pos < tokens.size())
    {
        const std::string& key = tokens[pos++];

        if (key == "offset")
        {
            if (!parseVec3(tokens, pos, entry.offset))
                return false;
        }
        else if (key == "layer")
        {
            if (pos >= tokens.size())
                return false;

            bool ok = false;
            entry.layer = layerFromString(tokens[pos++].c_str(), &ok,
                                          Layer::Infrastructure);
            if (!ok)
                return false;
        }
        else if (key == "profile")
        {
            if (pos >= tokens.size())
                return false;
            entry.profile = tokens[pos++];
        }
        else
        {
            return false;
        }
    }

    out.entries.insert({label, entry});
    return true;
}

} // namespace

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool loadColliderConfig(const std::string& path,
                        ColliderConfig& out,
                        std::string* error)
{
    std::ifstream file(path);
    if (!file)
    {
        if (error != nullptr)
            *error = "failed to open " + path;
        return false;
    }

    std::string line;
    while (std::getline(file, line))
    {
        // Комментарии
        const std::size_t comment = line.find('#');
        if (comment != std::string::npos)
            line.erase(comment);

        std::istringstream line_stream(line);
        std::vector<std::string> tokens;
        std::string token;
        while (line_stream >> token)
            tokens.push_back(std::move(token));

        if (tokens.empty())
            continue;

        if (!parseEntry(tokens, out))
            ++out.skipped_lines;
    }

    return true;
}

} // namespace collision
