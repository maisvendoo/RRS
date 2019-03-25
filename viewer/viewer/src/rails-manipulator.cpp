#include    "rails-manipulator.h"
#include    "math-funcs.h"

#include    <osgViewer/Viewer>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
RailsManipulator::RailsManipulator(QObject *parent)
    :   AbstractManipulator (parent)
    ,   pos_X0(0.0)
    ,   pos_Y0(0.0)
    ,   angle_H(0.0)
    ,   angle_V(0.0)
    ,   fixed(false)
    ,   rel_dp(osg::Vec3())
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
osg::Matrixd RailsManipulator::getMatrix() const
{
    osg::Matrix matrix = osg::Matrix::translate(osg::Vec3f(cp.driver_pos.x(), cp.driver_pos.z(), -cp.driver_pos.y()));
    matrix *= osg::Matrix::rotate(static_cast<double>(-cp.attitude.x()), osg::Vec3(1.0f, 0.0f, 0.0f));
    matrix *= osg::Matrix::rotate(static_cast<double>(-cp.attitude.z()), osg::Vec3(0.0f, 0.0f, 1.0f));
    matrix *= osg::Matrix::translate(cp.position);

    return matrix;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
osg::Matrixd RailsManipulator::getInverseMatrix() const
{
    osg::Matrix matrix = getMatrix();

    osg::Matrix invMatrix = osg::Matrix::inverse(matrix);

    invMatrix *= osg::Matrixf::rotate(angle_H, osg::Y_AXIS);
    invMatrix *= osg::Matrixf::rotate(angle_V, osg::X_AXIS);

    return invMatrix;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
RailsManipulator::~RailsManipulator()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void RailsManipulator::keysDownProcess(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void RailsManipulator::dragMouseProcess(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa)
{
    if (!fixed)
    {
        fixed = true;
        pos_X0 = ea.getX();
        pos_Y0 = ea.getY();
    }

    if (ea.getButtonMask() == osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON)
    {
        osgViewer::Viewer *viewer = static_cast<osgViewer::Viewer *>(&aa);
        osg::GraphicsContext *gc = viewer->getCamera()->getGraphicsContext();
        const osg::GraphicsContext::Traits *tr = gc->getTraits();

        float k1 = 2.0f;

        angle_H = k1 * (ea.getX() / tr->width - 0.5f);
        angle_V = k1 * (ea.getY() / tr->height - 0.5f);

        angle_V = cut(angle_V, -osg::PI_2f, osg::PI_2f);
    }
}

void RailsManipulator::releaseMouseProcess(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa)
{
    if (ea.getButton() == osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON)
    {
        fixed = false;
    }
}
