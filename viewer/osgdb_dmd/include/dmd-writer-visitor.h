#ifndef     DMD_WRITER_VISITOR_H
#define     DMD_WRITER_VISITOR_H

#include    <osg/NodeVisitor>
#include    <osg/Geometry>

typedef std::vector<std::string> dmd_content_t;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DMDWriterVisitor : public osg::NodeVisitor
{
public:

    DMDWriterVisitor(std::ostream &stream);

    virtual void apply(osg::Geometry &geom);

private:

    std::ostream &stream;

   dmd_content_t vertex_list;

   dmd_content_t face_list;

   dmd_content_t texcoord_list;

   dmd_content_t texface_list;

    void delete_dublicated_vertices(osg::Vec3Array *vertices);

    void create_dmd(int mode, osg::Geometry &geom);

    void process_triangles(osg::Geometry &geom, size_t step);

    void write_dmd(std::ostream &stream);
};

#endif // DMD_WRITER_VISITOR_H
