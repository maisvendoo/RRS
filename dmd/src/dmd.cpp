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

    if (!dmd_container.load(fin))
        return false;

    while (!dmd_container.isEnd())
    {
        std::string line = dmd_container.getLine();

        if (line == "New object")
            readNextMesh(dmd_container);

        if (line == "New Texture:")
            readTextureBlock(dmd_container);
    }

    return result;
}

dmd_mesh_t *DMDObject::getMesh()
{
    return &mesh;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void DMDObject::readNextMesh(DMDContainer &dmd_cont)
{
    std::string line = dmd_cont.getLine();

    while ( (line != "numverts numfaces") && (!dmd_cont.isEnd()) )
        line = dmd_cont.getLine();

    if (dmd_cont.isEnd())
        return;

    line = dmd_cont.getLine();

    std::vector<std::string> geom_data = parse_line(line);
    mesh.vertex_count = static_cast<size_t>(str_to_int(geom_data[0]));
    mesh.faces_count = static_cast<size_t>(str_to_int(geom_data[1]));

    line = dmd_cont.getLine();

    for (size_t i = 0; i < mesh.vertex_count; i++)
    {
        line = dmd_cont.getLine();

        std::string tmp = delete_symbol(line, '\t');
        std::vector<std::string> coords = parse_line(tmp);

        float x = str_to_float(coords[0]);
        float y = str_to_float(coords[1]);
        float z = str_to_float(coords[2]);

        mesh.vertices.push_back(osg::Vec3f(x, y, z));
    }

    line = dmd_cont.getLine();
    line = dmd_cont.getLine();

    for (size_t i = 0; i < mesh.faces_count; i++)
    {
        line = dmd_cont.getLine();

        std::string tmp = delete_symbol(line, '\t');
        std::vector<std::string> face_data = parse_line(tmp);

        face_t face;

        for (auto it = face_data.begin(); it != face_data.end(); ++it)
        {
            face.indices.push_back(str_to_int(*it) - 1);
        }

        calcFaceNormal(mesh, face);

        mesh.faces.push_back(face);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void DMDObject::readTextureBlock(DMDContainer &dmd_cont)
{
    std::string line = dmd_cont.getLine();
    line = dmd_cont.getLine();

    std::vector<std::string> tex_data = parse_line(line);
    mesh.tex_v_count = static_cast<size_t>(str_to_int(tex_data[0]));
    mesh.tex_f_count = static_cast<size_t>(str_to_int(tex_data[1]));

    line = dmd_cont.getLine();

    if (line != "Texture vertices:")
    {
        mesh.texture_present = false;
        return;
    }

    mesh.vertices_array = new osg::Vec3Array;
    mesh.smooth_normals = new osg::Vec3Array;
    mesh.texture_vertices = new osg::Vec2Array;

    mesh.vertices_array->resize(mesh.tex_v_count);
    mesh.smooth_normals->resize(mesh.tex_v_count);

    for (size_t i = 0; i < mesh.tex_v_count; ++i)
    {
        line = dmd_cont.getLine();

        std::string tmp = delete_symbol(line, '\t');
        std::vector<std::string> tex_vertex = parse_line(tmp);

        float x = str_to_float(tex_vertex[0]);
        float y = str_to_float(tex_vertex[1]);
        float z = str_to_float(tex_vertex[2]);

        mesh.texture_vertices->push_back(osg::Vec2f(x, 1.0f - y));
    }

    line = dmd_cont.getLine();
    line = dmd_cont.getLine();

    for (size_t i = 0; i < mesh.tex_f_count; ++i)
    {
        line = dmd_cont.getLine();
        std::string tmp = delete_symbol(line, '\t');
        std::vector<std::string> face_data = parse_line(tmp);

        face_t face;

        for (auto it = face_data.begin(); it != face_data.end(); ++it)
        {
            int tv_idx = str_to_int(*it) - 1;
            face.indices.push_back(tv_idx);

            face_t f = mesh.faces[i];
            int v_idx = f.indices[face.indices.size() - 1];

            mesh.vertices_array->at(tv_idx).set(mesh.vertices[v_idx]);
            osg::Vec3f n = mesh.smooth_normals->at(tv_idx);
            n = n + f.normal;
            mesh.smooth_normals->at(tv_idx).set(n);
        }

        mesh.texture_faces.push_back(face);
    }

    mesh.texture_present = true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void DMDObject::calcFaceNormal(const dmd_mesh_t &mesh, face_t &face)
{
    osg::Vec3f vec1 = mesh.vertices[face.indices[0]];
    osg::Vec3f vec2 = mesh.vertices[face.indices[1]];
    osg::Vec3f vec3 = mesh.vertices[face.indices[2]];

    osg::Vec3f v12 = vec1 - vec2;
    osg::Vec3f v23 = vec2 - vec3;

    osg::Vec3f n = v12 ^ v23;
    face.normal = n *= (1 / n.length());
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void DMDObject::calcSmoothNormales(dmd_mesh_t &mesh)
{
    /*mesh.smooth_normals = new osg::Vec3Array;
    mesh.smooth_normals->resize(mesh.vertex_count);

    for (size_t i = 0; i < mesh.faces_count; i++)
    {
        std::vector<int> indices = mesh.faces[i].indices;

        for (size_t j = 0; j < indices.size(); j++)
        {
            osg::Vec3f n = mesh.smooth_normals->at(indices[j]);
            n = n + mesh.faset_normals->at(i);
            n = n *= (1/n.length());
            mesh.smooth_normals->at(indices[j]).set(n);
        }
    }*/
}
