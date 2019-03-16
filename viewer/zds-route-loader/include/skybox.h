#ifndef     SKYBOX_H
#define     SKYBOX_H

#include    <osg/MatrixTransform>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class Skybox : public osg::Transform
{
public:

    Skybox();

    Skybox(const Skybox &copy, osg::CopyOp copyop = osg::CopyOp::SHALLOW_COPY);

    META_Node(osg, Skybox)

    virtual bool computeLocalToWorldMatrix(osg::Matrix &matrix, osg::NodeVisitor *nv) const;

    virtual bool computeWorldToLocalMatrix(osg::Matrix &matrix, osg::NodeVisitor *nv) const;

    void load(const std::string &routeDir, osg::Group *scene);

protected:

    virtual ~Skybox();

    osg::Image *loadImage(const std::string &path);

    osg::Node *loadSkyModel(const std::string &path);
};

#endif // SKYBOX_H
