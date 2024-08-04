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
dmd_multimesh_t load_dmd(std::ifstream &stream)
{
    dmd_multimesh_t multimesh;

    if (stream.is_open())
    {
        while (!stream.eof())
        {
            std::string line = getLine(stream);

            if (line == "New object")
            {
                readNextMesh(stream, multimesh);
            }

            if (line == "New Texture:")
            {
                readTextureBlock(stream, multimesh);
            }
        }
    }

    return multimesh;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void readNextMesh(std::ifstream &stream, dmd_multimesh_t &multimesh)
{
    dmd_mesh_t mesh;

    std::string line = getLine(stream);

    // Определяем число вершин и число граней
    while ( (line != "numverts numfaces") && (!stream.eof()))
        line = getLine(stream);

    if (stream.eof())
        return;

    line = getLine(stream);
    std::istringstream iss(line);
    iss >> mesh.vertex_count >> mesh.faces_count;

    line = getLine(stream);

    // Читаем массив вершин
    mesh.vertices = new osg::Vec3Array;    

    for (unsigned int i = 0; i < mesh.vertex_count; ++i)
    {
        line = getLine(stream);
        line = delete_symbol(line, '\t');
        osg::Vec3 point;

        std::istringstream ss(line);
        ss >> point.x() >> point.y() >> point.z();

        mesh.vertices->push_back(point);
    }

    std::string empty_line = getLine(stream);
    line = getLine(stream);

    mesh.vertex_normals = new osg::Vec3Array;

    for (unsigned int i = 0; i < mesh.faces_count; ++i)
    {
        line = getLine(stream);
        line = delete_symbol(line, '\t');        

        std::istringstream ss(line);

        face_t face;

        while (!ss.eof())
        {
            unsigned int idx = 0;
            ss >> idx;
            face.push_back(idx-1);
        }

        mesh.faces.push_back(face);

        osg::Vec3 normal = mesh.calcFaceNormal(face);

        for (size_t j = 0; j < face.size(); ++j)
            mesh.vertex_normals->push_back(normal);
    }

    multimesh.meshes.push_back(mesh);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void readTextureBlock(std::ifstream &stream, dmd_multimesh_t &multimesh)
{
    std::string line = getLine(stream);
    line = getLine(stream);

    std::istringstream iss(line);
    iss >> multimesh.tex_v_count >> multimesh.tex_f_count;

    line = getLine(stream);

    if (line != "Texture vertices:")
    {
        multimesh.is_texture_present = false;
        return;
    }

    multimesh.is_texture_present = true;

    // Текстурные координаты
    multimesh.texvrtices = new osg::Vec2Array;

    for (unsigned int i = 0; i < multimesh.tex_v_count; ++i)
    {
        line = delete_symbol(getLine(stream), '\t');
        std::istringstream ss(line);

        osg::Vec2 texel;
        float z = 0;
        ss >> texel.x() >> texel.y() >> z;

        texel.y() = 1.0f - texel.y();

        multimesh.texvrtices->push_back(texel);
    }    

    line = getLine(stream);
    line = getLine(stream);

    for (unsigned int i = 0; i < multimesh.tex_f_count; ++i)
    {
        line = delete_symbol(getLine(stream), '\t');
        std::istringstream ss(line);        


        face_t texface;

        while (!ss.eof())
        {            
            unsigned int idx = 0;
            ss >> idx;            

            texface.push_back(idx-1);
        }

        multimesh.texfaces.push_back(texface);
    }    
}

