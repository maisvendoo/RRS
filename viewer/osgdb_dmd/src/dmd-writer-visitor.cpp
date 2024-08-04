#include    "dmd-writer-visitor.h"

#include    <sstream>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
DMDWriterVisitor::DMDWriterVisitor(std::ostream &stream) :
    osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
  , stream(stream)

{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void DMDWriterVisitor::apply(osg::Geometry &geom)
{
    for (size_t i = 0; i < geom.getNumPrimitiveSets(); ++i)
    {
        osg::PrimitiveSet *prim_set = geom.getPrimitiveSet(i);

        int mode = prim_set->getMode();

        create_dmd(mode, geom);

        if (vertex_list.empty())
            break;

        write_dmd(stream);
    }

    traverse(geom);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void DMDWriterVisitor::delete_dublicated_vertices(osg::Vec3Array *vertices)
{
    for (size_t j = 0; j < vertices->getNumElements(); ++j)
    {
        osg::Vec3f prev = vertices->at(j);

        for (size_t i = j + 1; i < vertices->getNumElements(); ++i)
        {
            osg::Vec3f curr = vertices->at(i);

            if (prev == curr)
            {
                osg::Vec3Array::iterator it = vertices->begin();
                std::advance(it, i);
                vertices->erase(it);
            }
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void DMDWriterVisitor::create_dmd(int mode, osg::Geometry &geom)
{
    switch (mode)
    {
        case GL_TRIANGLES:
        {
            process_triangles(geom, 3);
            break;
        }

        case GL_TRIANGLE_STRIP:
        {
            process_triangles(geom, 1);
            break;
        }

        default:

            break;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void DMDWriterVisitor::process_triangles(osg::Geometry &geom, size_t step)
{
    osg::Vec3Array *vertices = dynamic_cast<osg::Vec3Array *>(geom.getVertexArray());

    for (size_t i = 0; i < vertices->getNumElements(); ++i)
    {
        std::stringstream v1_ss;

        v1_ss << "\t" << vertices->at(i).x() << " "
             << vertices->at(i).y() << " "
             << vertices->at(i).z();

        vertex_list.push_back(v1_ss.str());
    }

    for (size_t i = 0; i <= vertices->getNumElements() - step; i += step)
    {
        std::stringstream f_ss;

        f_ss << "\t" << i + 1 << " "
             << i + 2 << " "
             << i + 3;

        face_list.push_back(f_ss.str());
    }

    osg::Vec2Array *texcoords = dynamic_cast<osg::Vec2Array *>(geom.getTexCoordArray(0));

    if (texcoords == nullptr)
        return;

    for (size_t i = 0; i < texcoords->getNumElements(); ++i)
    {
        std::stringstream t1_ss;

        t1_ss << "\t" << texcoords->at(i).x() << " "
              << 1.0 - texcoords->at(i).y() << " 0.0";

        texcoord_list.push_back(t1_ss.str());
    }


    for (size_t i = 0; i <= texcoords->getNumElements() - step; i += step)
    {
        std::stringstream tf_ss;

        tf_ss << "\t" << i + 1 << " "
              << i + 2 << " "
              << i + 3;

        texface_list.push_back(tf_ss.str());
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void DMDWriterVisitor::write_dmd(std::ostream &stream)
{
    size_t numverts = vertex_list.size();

    size_t numfaces = face_list.size();

    stream << "New object" << std::endl
           << "TriMesh()" << std::endl
           << "numverts numfaces" << std::endl
           << "\t" << numverts << "\t" << numfaces << std::endl
           << "Mesh vertices:" << std::endl;

    for (size_t i = 0; i < vertex_list.size(); ++i)
    {
        stream << vertex_list[i] << std::endl;
    }

    stream << "end vertices" << std::endl
           << "Mesh faces:" << std::endl;

    for (size_t i = 0; i < face_list.size(); ++i)
    {
        stream << face_list[i] << std::endl;
    }

    stream << "end faces" << std::endl
           << "end mesh" << std::endl;


    if (!texcoord_list.empty())
    {
        stream << "New Texture:" << std::endl
               << "numtverts numtvfaces" << std::endl
               << "\t" << texcoord_list.size() << "\t"
               << texface_list.size() << std::endl
               << "Texture vertices:" << std::endl;

        for (size_t i = 0; i < texcoord_list.size(); ++i)
        {
            stream << texcoord_list[i] << std::endl;
        }

        stream << "end texture vertices" << std::endl
               << "Texture faces:" << std::endl;

        for (size_t i = 0; i < texface_list.size(); ++i)
        {
            stream << texface_list[i] << std::endl;
        }

        stream << "end texture faces" << std::endl
               << "end of texture" << std::endl;
    }

    stream << "end of file" << std::endl;
}
