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
    ,   rel_dp(osg::Vec3())
    ,   move_height(false)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
osg::Matrixd RailsManipulator::getMatrix() const
{
    osg::Matrix matrix = osg::Matrix::translate(osg::Vec3f(cp.driver_pos.x() + rel_dp.x(),
                                                           cp.driver_pos.z() + rel_dp.z(),
                                                           -cp.driver_pos.y() - rel_dp.y()));

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
    float speed = 2.0f;

    switch (ea.getKey())
    {
    case osgGA::GUIEventAdapter::KEY_Left:

        rel_dp.x() -= speed * delta_time;
        break;

    case osgGA::GUIEventAdapter::KEY_Right:

        rel_dp.x() += speed * delta_time;
        break;

    case osgGA::GUIEventAdapter::KEY_Up:

        rel_dp.y() += speed * delta_time;
        break;


    case osgGA::GUIEventAdapter::KEY_Down:

        rel_dp.y() -= speed * delta_time;
        break;
    }    
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void RailsManipulator::dragMouseProcess(const osgGA::GUIEventAdapter &ea,
                                        osgGA::GUIActionAdapter &aa)
{
    if (ea.getButtonMask() == osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON)
    {
        float k1 = 2.0f;

        float pos_X = ea.getXnormalized();
        float pos_Y = ea.getYnormalized();

        angle_H = k1 * (pos_X - pos_X0);
        angle_V = k1 * (pos_Y - pos_Y0);

        angle_V = cut(angle_V, -osg::PI_2f, osg::PI_2f);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void RailsManipulator::pushMouseProcess(const osgGA::GUIEventAdapter &ea,
                                        osgGA::GUIActionAdapter &aa)
{
    if (ea.getButton() == osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)
    {
        osgViewer::Viewer *viewer = static_cast<osgViewer::Viewer *>(&aa);
        osg::Camera *camera = viewer->getCamera();
        osg::GraphicsContext *gc = camera->getGraphicsContext();
        const osg::GraphicsContext::Traits *tr = gc->getTraits();
        double aspectRatio = static_cast<double>(tr->width) / static_cast<double>(tr->height);

        camera->setProjectionMatrixAsPerspective(settings.fovy, aspectRatio, settings.zNear, settings.zFar);

        angle_H = angle_V = 0.0;
        rel_dp = osg::Vec3();
    }

    if (ea.getButton() == osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON)
    {
        if (!fixed)
        {
            fixed = true;
            pos_X0 = ea.getXnormalized();
            pos_Y0 = ea.getYnormalized();
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void RailsManipulator::releaseMouseProcess(const osgGA::GUIEventAdapter &ea,
                                           osgGA::GUIActionAdapter &aa)
{
    fixed = false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void RailsManipulator::scrollProcess(const osgGA::GUIEventAdapter &ea,
                                     osgGA::GUIActionAdapter &aa)
{
    osgViewer::Viewer *viewer = static_cast<osgViewer::Viewer *>(&aa);
    osg::Camera *camera = viewer->getCamera();

    double fovy = 0;
    double aspectRatio = 0;
    double zNear = 0;
    double zFar = 0;

    camera->getProjectionMatrixAsPerspective(fovy, aspectRatio, zNear, zFar);

    osgGA::GUIEventAdapter::ScrollingMotion sm = ea.getScrollingMotion();

    double step = 1.0;
    float speed = 2.0f;

    switch (sm)
    {
    case osgGA::GUIEventAdapter::SCROLL_UP:

        if (  (ea.getModKeyMask() == osgGA::GUIEventAdapter::MODKEY_LEFT_CTRL) ||
              (ea.getModKeyMask() == osgGA::GUIEventAdapter::MODKEY_RIGHT_CTRL))
        {
            rel_dp.z() += speed * delta_time;
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
            rel_dp.z() -= speed * delta_time;
        }
        else
        {
            fovy += step;
        }

        break;
    }

    camera->setProjectionMatrixAsPerspective(fovy, aspectRatio, zNear, zFar);
}
