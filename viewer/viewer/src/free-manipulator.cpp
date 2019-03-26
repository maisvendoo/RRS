#include    "free-manipulator.h"
#include    "math-funcs.h"

#include    <osgViewer/Viewer>

#include    <iostream>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
FreeManipulator::FreeManipulator(settings_t settings, QObject *parent)
    : AbstractManipulator(parent)
    , settings(settings)
    , init_pos(camera_position_t())
    , rel_pos(osg::Vec3(2.5, 0.0, -2.0))
    , angle_H(0.0f)
    , angle_V(0.0f)
    , pos_X0(0.0f)
    , pos_Y0(0.0f)
    , fixed(false)
    , forward(osg::Vec3())
    , traverse(osg::Vec3())
    , camera(nullptr)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
osg::Matrixd FreeManipulator::getMatrix() const
{
    osg::Matrix matrix = osg::Matrix::translate(osg::Vec3f(init_pos.driver_pos.x() + rel_pos.x(),
                                                           init_pos.driver_pos.z() + rel_pos.z(),
                                                           -init_pos.driver_pos.y() - rel_pos.y()));
    float a_x = -init_pos.attitude.x();
    float a_z = -init_pos.attitude.z();

    matrix *= osg::Matrix::rotate(static_cast<double>(a_x), osg::Vec3(1.0f, 0.0f, 0.0f));
    matrix *= osg::Matrix::rotate(static_cast<double>(a_z), osg::Vec3(0.0f, 0.0f, 1.0f));
    matrix *= osg::Matrix::translate(init_pos.position);

    return matrix;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
osg::Matrixd FreeManipulator::getInverseMatrix() const
{
    osg::Matrix invMatrix = osg::Matrix::inverse(getMatrix());

    invMatrix *= osg::Matrixf::rotate(angle_H, osg::Y_AXIS);
    invMatrix *= osg::Matrixf::rotate(angle_V, osg::X_AXIS);

    return invMatrix;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void FreeManipulator::init(const osgGA::GUIEventAdapter &ea,
                           osgGA::GUIActionAdapter &aa)
{
    Q_UNUSED(ea)
    Q_UNUSED(aa)

    init_pos = cp;

    osgViewer::Viewer *viewer = static_cast<osgViewer::Viewer *>(&aa);
    camera = viewer->getCamera();    
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool FreeManipulator::performMovementRightMouseButton(const double eventTimeDelta,
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
bool FreeManipulator::handleKeyDown(const osgGA::GUIEventAdapter &ea,
                                    osgGA::GUIActionAdapter &aa)
{
    osg::Vec3 eye, center, up;
    camera->getViewMatrixAsLookAt(eye, center, up);

    osg::Vec3 front = center - eye;
    front.z() = 0;
    front.normalize();

    osg::Vec3 traverse = front ^ up;
    traverse.normalize();

    float V = 5.0f;

    if ( (ea.getModKeyMask() == osgGA::GUIEventAdapter::MODKEY_LEFT_SHIFT) ||
         (ea.getModKeyMask() == osgGA::GUIEventAdapter::MODKEY_RIGHT_SHIFT) )
    {
        V = 10.0f * V;
    }

    switch (ea.getKey())
    {
    case osgGA::GUIEventAdapter::KEY_Up:
        {
            rel_pos += front * (V * delta_time);
            break;
        }

    case osgGA::GUIEventAdapter::KEY_Down:
        {
            rel_pos -= front * (V * delta_time);
            break;
        }

    case osgGA::GUIEventAdapter::KEY_Left:
        {
            rel_pos -= traverse * (V * delta_time);
            break;
        }

    case osgGA::GUIEventAdapter::KEY_Right:
        {
            rel_pos += traverse * (V * delta_time);
            break;
        }
    }

    return false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool FreeManipulator::handleMouseWheel(const osgGA::GUIEventAdapter &ea,
                                       osgGA::GUIActionAdapter &aa)
{
    double fovy = 0;
    double aspectRatio = 0;
    double zNear = 0;
    double zFar = 0;

    camera->getProjectionMatrixAsPerspective(fovy, aspectRatio, zNear, zFar);

    osgGA::GUIEventAdapter::ScrollingMotion sm = ea.getScrollingMotion();

    double step = 1.0;
    float speed = 5.0f;

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

    camera->setProjectionMatrixAsPerspective(fovy, aspectRatio, zNear, zFar);

    return false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool FreeManipulator::handleMousePush(const osgGA::GUIEventAdapter &ea,
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

        rel_pos = osg::Vec3(osg::Vec3(2.5, 0.0, -2.0));
        angle_H = angle_V = 0;
    }

    return false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
FreeManipulator::~FreeManipulator()
{

}

