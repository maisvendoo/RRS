#include    "free-manipulator.h"

#include    <osgViewer/Viewer>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
FreeManipulator::FreeManipulator(settings_t settings, QObject *parent)
    : AbstractManipulator(parent)
    , settings(settings)
    , init_pos(camera_position_t())
    , rel_pos(settings.free_cam_init_pos)
    , angle_H(0.0f)
    , angle_V(0.0f)
    , camera(nullptr)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
osg::Matrixd FreeManipulator::getMatrix() const
{
    osg::Matrixd matrix = osg::Matrixd::translate(osg::Vec3d( rel_pos.x(),
                                                              rel_pos.z(),
                                                              -rel_pos.y() ));

    matrix *= osg::Matrixd::rotate(-init_pos.attitude.x(), osg::Vec3d(1.0f, 0.0f, 0.0f));
    matrix *= osg::Matrixd::rotate(-init_pos.attitude.z(), osg::Vec3d(0.0f, 0.0f, 1.0f));
    matrix *= osg::Matrixd::translate(init_pos.position);

    return matrix;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
osg::Matrixd FreeManipulator::getInverseMatrix() const
{
    osg::Matrixd invMatrix = osg::Matrixd::inverse(getMatrix());

    invMatrix *= osg::Matrixd::rotate(angle_H, osg::Y_AXIS);
    invMatrix *= osg::Matrixd::rotate(angle_V, osg::X_AXIS);

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
    Q_UNUSED(eventTimeDelta)

    double k1 = static_cast<double>(settings.free_cam_rot_coeff);

    angle_H += static_cast<float>(k1 * dx);
    angle_V -= static_cast<float>(k1 * dy);

    return false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool FreeManipulator::handleKeyDown(const osgGA::GUIEventAdapter &ea,
                                    osgGA::GUIActionAdapter &aa)
{
    Q_UNUSED(aa)

    osg::Vec3d eye, center, up;
    camera->getViewMatrixAsLookAt(eye, center, up, 100.0);

    osg::Vec3d front = center - eye;
    front.z() = 0;
    front.normalize();

    osg::Vec3d traverse = front ^ up;
    traverse.normalize();

    double V = settings.free_cam_speed;

    if ( (ea.getModKeyMask() == osgGA::GUIEventAdapter::MODKEY_LEFT_SHIFT) ||
         (ea.getModKeyMask() == osgGA::GUIEventAdapter::MODKEY_RIGHT_SHIFT) )
    {
        V = settings.free_cam_speed_coeff * V;
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
    Q_UNUSED(aa)

    double fovy = 0;
    double aspectRatio = 0;
    double zNear = 0;
    double zFar = 0;

    camera->getProjectionMatrixAsPerspective(fovy, aspectRatio, zNear, zFar);

    osgGA::GUIEventAdapter::ScrollingMotion sm = ea.getScrollingMotion();

    double step = settings.free_cam_fovy_step;
    double fovy_min = settings.fovy_min;
    double fovy_max = settings.fovy_max;

    float speed = settings.free_cam_speed;
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
            if (fovy < fovy_min)
                fovy = fovy_min;
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
            if (fovy > fovy_max)
                fovy = fovy_max;
        }

        break;

    default:

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

        rel_pos = osg::Vec3d(settings.free_cam_init_pos);
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

