#include    "dmd.h"
#include    "dmd-mesh.h"
#include    "dmd_parser.h"

#include    <osg/Geometry>

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
dmd_multymesh_t DMDObject::getMultyMesh() const
{
    return multyMesh;
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
            face.indices.push_back(str_to_int(*it) - 1);
        }

        mesh->faces.push_back(face);
    }

    calcNormales(mesh);
    calcSmoothNormales(mesh);

    multyMesh.meshes.push_back(mesh);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void DMDObject::readTextureBlock(std::ifstream &fin)
{
    std::string line;

    std::getline(fin, line);
    std::getline(fin, line);

    std::vector<std::string> tex_data = parse_line(line);

    multyMesh.tex_v_count = static_cast<size_t>(str_to_int(tex_data[0]));
    multyMesh.tex_f_count = static_cast<size_t>(str_to_int(tex_data[1]));

    std::getline(fin, line);

    if (line != "Texture vertices:")
    {
        multyMesh.texture_present = false;
        return;
    }

    multyMesh.texture_vertices = new osg::Vec3Array;

    // Read texture vertices
    for (size_t i = 0; i < multyMesh.tex_v_count; i++)
    {
        std::getline(fin, line);
        std::string tmp = delete_symbol(line, '\t');
        std::vector<std::string> tex_vertex = parse_line(tmp);

        float x = str_to_float(tex_vertex[0]);
        float y = str_to_float(tex_vertex[1]);
        float z = str_to_float(tex_vertex[2]);

        if (x > multyMesh.tx_max)
            multyMesh.tx_max = x;

        if (y > multyMesh.ty_max)
            multyMesh.ty_max = y;

        multyMesh.texture_vertices->push_back(osg::Vec3(x, y, z));
    }

    std::getline(fin, line);
    std::getline(fin, line);

    // Read texture faces
    for (size_t i = 0; i < multyMesh.tex_f_count; i++)
    {
        std::getline(fin, line);
        std::string tmp = delete_symbol(line, '\t');
        std::vector<std::string> face_data = parse_line(tmp);

        face_t face;

        for (auto it = face_data.begin(); it != face_data.end(); ++it)
        {
            face.indices.push_back(str_to_int(*it) - 1);
        }

        multyMesh.texture_faces.push_back(face);
    }

    multyMesh.texture_present = true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void DMDObject::calcNormales(dmd_mesh_t *mesh)
{
    mesh->faset_normals = new osg::Vec3Array;

    for (auto face_it = mesh->faces.begin(); face_it != mesh->faces.end(); ++face_it)
    {
        std::vector<int> indices = (*face_it).indices;

        osg::Vec3f vec1 = mesh->vertices->at(indices[0]);
        osg::Vec3f vec2 = mesh->vertices->at(indices[1]);
        osg::Vec3f vec3 = mesh->vertices->at(indices[2]);

        osg::Vec3f v12 = vec1 - vec2;
        osg::Vec3f v23 = vec2 - vec3;

        osg::Vec3f n = v12 ^ v23;
        float length = n.length();

        mesh->faset_normals->push_back(n *= (1 / length));
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void DMDObject::calcSmoothNormales(dmd_mesh_t *mesh)
{
    mesh->smooth_normals = new osg::Vec3Array;

    for (size_t i = 0; i < mesh->vertex_count; i++)
    {
        mesh->smooth_normals->push_back(osg::Vec3f(0, 0, 0));
    }

    for (size_t i = 0; i < mesh->faces_count; i++)
    {
        std::vector<int> indices = mesh->faces[i].indices;

        for (size_t j = 0; j < indices.size(); j++)
        {
            osg::Vec3f n = mesh->smooth_normals->at(indices[j]);
            n = n + mesh->faset_normals->at(i);
            mesh->smooth_normals->at(indices[j]).set(n);
        }
    }

    for (size_t i = 0; i < mesh->vertex_count; i++)
    {
        osg::Vec3f n = mesh->smooth_normals->at(i);
        float length = n.length();
        n = n *= (1/ length);
        mesh->smooth_normals->at(i).set(n);
    }
}
