#include    "dmd.h"
#include    "dmd-mesh.h"
#include    "dmd_parser.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
DMDObject::DMDObject(std::string file_path)
    : line_number(1)
{
    if (!file_path.empty())
        this->load(file_path);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
DMDObject::~DMDObject()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool DMDObject::load(const std::string &file_path)
{
    std::ifstream fin(file_path);
    return load(fin);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool DMDObject::load(std::ifstream &fin)
{
    bool result = true;

    if (fin.is_open())
    {
        std::string line;

        while (!fin.eof())
        {
            std::getline(fin, line);

            if (line == "New object")
                readNextMesh(fin);

            if (line == "New Texture:")
                readTextureBlock(fin);

            line_number++;
        }

        fin.close();
    }

    return result;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::vector<dmd_mesh_t *> DMDObject::getMeshes() const
{
    return meshes;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void DMDObject::readNextMesh(std::ifstream &fin)
{
    std::string line;
    std::getline(fin, line);

    while ( (line != "numverts numfaces") && (!fin.eof()) )
        std::getline(fin, line);

    if (fin.eof())
        return;

    std::getline(fin, line);

    dmd_mesh_t *mesh = new dmd_mesh_t();

    std::vector<std::string> geom_data = parse_line(line);
    mesh->vertex_count = static_cast<size_t>(str_to_int(geom_data[0]));
    mesh->faces_count = static_cast<size_t>(str_to_int(geom_data[1]));

    std::getline(fin, line);

    mesh->vertices = new osg::Vec3Array;

    for (size_t i = 0; i < mesh->vertex_count; i++)
    {
        std::getline(fin, line);
        std::string tmp = delete_symbol(line, '\t');
        std::vector<std::string> coords = parse_line(tmp);

        float x = str_to_float(coords[0]);
        float y = str_to_float(coords[1]);
        float z = str_to_float(coords[2]);

        mesh->vertices->push_back(osg::Vec3(x, y, z));
    }

    std::getline(fin, line);
    std::getline(fin, line);

    for (size_t i = 0; i < mesh->faces_count; i++)
    {
        std::getline(fin, line);
        std::string tmp = delete_symbol(line, '\t');
        std::vector<std::string> face_data = parse_line(tmp);

        face_t face;

        for (auto it = face_data.begin(); it != face_data.end(); ++it)
        {
            face.indices.push_back(str_to_int(*it));
        }

        mesh->faces.push_back(face);
    }

    meshes.push_back(mesh);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void DMDObject::readTextureBlock(std::ifstream &fin)
{

}
