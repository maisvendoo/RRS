#include    "dmd-parser.h"

#include    <algorithm>
#include    <sstream>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::string delete_symbol(const std::string &str, char symbol)
{
    std::string tmp = str;
    tmp.erase(std::remove(tmp.begin(), tmp.end(), symbol), tmp.end());
    return tmp;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::string getLine(std::ifstream &stream)
{
    std::string line;
    std::getline(stream, line);

    return delete_symbol(line, '\r');
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
dmd_mesh_t load_dmd(std::ifstream &stream)
{
    dmd_mesh_t mesh;

    if (stream.is_open())
    {
        while (!stream.eof())
        {
            std::string line = getLine(stream);

            if (line == "New object")
            {
                readNextMesh(stream, mesh);
            }

            if (line == "New Texture:")
            {
                readTextureBlock(stream, mesh);
            }
        }
    }

    return mesh;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void readNextMesh(std::ifstream &stream, dmd_mesh_t &mesh)
{
    std::string line = getLine(stream);

    while ( (line != "numverts numfaces") && (!stream.eof()))
        line = getLine(stream);

    if (stream.eof())
        return;

    line = getLine(stream);
    int numverts = 0;
    int numfaces = 0;
    std::istringstream iss(line);
    iss >> numverts >> numfaces;

    line = getLine(stream);

    mesh.vertices = new osg::Vec3Array;

    for (int i = 0; i < numverts; ++i)
    {
        line = getLine(stream);
        line = delete_symbol(line, '\t');
        osg::Vec3 point;

        std::istringstream ss(line);
        ss >> point.x() >> point.y() >> point.z();

        mesh.vertices->push_back(point);
    }

    line = getLine(stream);
    line = getLine(stream);

    for (int i = 0; i < numfaces; ++i)
    {
        line = getLine(stream);
        line = delete_symbol(line, '\t');

        osg::ref_ptr<osg::DrawElementsUInt> face = new osg::DrawElementsUInt(osg::PrimitiveSet::POLYGON, 0);

        std::istringstream ss(line);

        while (!ss.eof())
        {
            unsigned int idx = 0;
            ss >> idx;
            face->push_back(idx - 1);
        }

        mesh.faces.push_back(face);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void readTextureBlock(std::ifstream &stream, dmd_mesh_t &mesh)
{
    std::string line = getLine(stream);
    line = getLine(stream);

    std::istringstream iss(line);
    unsigned int tex_v_count = 0;
    unsigned int tex_f_count = 0;
    iss >> tex_v_count >> tex_f_count;

    line = getLine(stream);

    if (line != "Texture vertices:")
    {
        mesh.is_texture_present = false;
        return;
    }

    mesh.is_texture_present = true;

    mesh.texcoords = new osg::Vec2Array;    

    for (unsigned int i = 0; i < tex_v_count; ++i)
    {
        line = delete_symbol(getLine(stream), '\t');
        std::istringstream ss(line);

        osg::Vec2 texel;
        float z = 0;
        ss >> texel.x() >> texel.y() >> z;

        mesh.texcoords->push_back(texel);
    }    

    line = getLine(stream);
    line = getLine(stream);

    mesh.texvertices = new osg::Vec3Array;
    mesh.texvertices->resize(tex_v_count);

    for (unsigned int i = 0; i < tex_f_count; ++i)
    {
        line = delete_symbol(getLine(stream), '\t');
        std::istringstream ss(line);

        osg::ref_ptr<osg::DrawElementsUInt> texface = new osg::DrawElementsUInt(osg::PrimitiveSet::POLYGON, 0);

        while (!ss.eof())
        {
            unsigned long v_idx = mesh.faces[i]->at(texface->size());
            unsigned int idx = 0;
            ss >> idx;
            texface->push_back(idx - 1);
            mesh.texvertices->at(idx - 1).set(mesh.vertices->at(v_idx));
        }

        mesh.texfaces.push_back(texface);
    }
}
