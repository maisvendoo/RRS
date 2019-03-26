#include    "rails-manipulator.h"
#include    "math-funcs.h"

#include    <osgViewer/Viewer>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
RailsManipulator::RailsManipulator(settings_t settings, QObject *parent)
    :   AbstractManipulator (parent)
    ,   settings(settings)
    ,   pos_X0(0.0)
    ,   pos_Y0(0.0)
    ,   angle_H(0.0)
    ,   angle_V(0.0)
    ,   fixed(false)
    ,   rel_pos(osg::Vec3())
    ,   move_height(false)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
osg::Matrixd RailsManipulator::getMatrix() const
{
    osg::Matrix matrix = osg::Matrix::translate(osg::Vec3f(cp.driver_pos.x() + rel_pos.x(),
                                                           cp.driver_pos.z() + rel_pos.z(),
                                                           -cp.driver_pos.y() - rel_pos.y()));

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
bool RailsManipulator::performMovementRightMouseButton(const double eventTimeDelta,
                                                       const double dx,
                                                       const double dy)
{
    double k1 = 1.0;

    angle_H += static_cast<float>(k1 * dx);
    angle_V += static_cast<float>(k1 * dy);

    return false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool RailsManipulator::handleMouseWheel(const osgGA::GUIEventAdapter &ea,
                                       osgGA::GUIActionAdapter &aa)
{
    double fovy = 0;
    double aspectRatio = 0;
    double zNear = 0;
    double zFar = 0;

    osgViewer::Viewer *viewer = static_cast<osgViewer::Viewer *>(&aa);
    viewer->getCamera()->getProjectionMatrixAsPerspective(fovy, aspectRatio, zNear, zFar);

    osgGA::GUIEventAdapter::ScrollingMotion sm = ea.getScrollingMotion();

    double step = 1.0;
    float speed = 2.0f;

    switch (sm)
    {
    case osgGA::GUIEventAdapter::SCROLL_UP:

        if (  (ea.getModKeyMask() == osgGA::GUIEventAdapter::MODKEY_LEFT_CTRL) ||
              (ea.getModKeyMask() == osgGA::GUIEventAdapter::MODKEY_RIGHT_CTRL))
        {
            rel_pos.z() += speed * delta_time;
        }
        else
        {
            fovy -= step;
        }

        break;

    case osgGA::GUIEventAdapter::SCROLL_DOWN:

        if (  (ea.getModKeyMask() == osgGA::GUIEventAdapter::MODKEY_LEFT_CTRL) ||
              (ea.getModKeyMask() == osgGA::GUIEventAdapter::MODKEY_RIGHT_CTRL))
        {
            rel_pos.z() -= speed * delta_time;
        }
        else
        {
            fovy += step;
        }

        break;
    }

    viewer->getCamera()->setProjectionMatrixAsPerspective(fovy, aspectRatio, zNear, zFar);

    return false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool RailsManipulator::handleKeyDown(const osgGA::GUIEventAdapter &ea,
                                    osgGA::GUIActionAdapter &aa)
{
    float V = 2.0f;

    switch (ea.getKey())
    {
    case osgGA::GUIEventAdapter::KEY_Up:
        {
            rel_pos.y() += V * delta_time;
            break;
        }

    case osgGA::GUIEventAdapter::KEY_Down:
        {
            rel_pos.y() -= V * delta_time;
            break;
        }

    case osgGA::GUIEventAdapter::KEY_Left:
        {
            rel_pos.x() -= V * delta_time;
            break;
        }

    case osgGA::GUIEventAdapter::KEY_Right:
        {
            rel_pos.x() += V * delta_time;
            break;
        }
    }

    return false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool RailsManipulator::handleMousePush(const osgGA::GUIEventAdapter &ea,
                                       osgGA::GUIActionAdapter &aa)
{
    if (ea.getButton() == osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)
    {
        osgViewer::Viewer *viewer = static_cast<osgViewer::Viewer *>(&aa);
        osg::Camera *camera = viewer->getCamera();
        const osg::GraphicsContext::Traits *tr = camera->getGraphicsContext()->getTraits();

        double aspectRatio = static_cast<double>(tr->width) / static_cast<double>(tr->height);

        camera->setProjectionMatrixAsPerspective(settings.fovy,
                                                 aspectRatio,
                                                 settings.zNear,
                                                 settings.zFar);

        rel_pos = osg::Vec3();
        angle_H = angle_V = 0;
    }

    return false;
}
