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
    , rel_pos(osg::Vec3(2.5, 0.0, -1.0))
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
FreeManipulator::~FreeManipulator()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void FreeManipulator::keysDownProcess(const osgGA::GUIEventAdapter &ea,
                                      osgGA::GUIActionAdapter &aa)
{
    float V = 5.0;

    if ( (ea.getModKeyMask() == osgGA::GUIEventAdapter::MODKEY_LEFT_SHIFT) ||
         (ea.getModKeyMask() == osgGA::GUIEventAdapter::MODKEY_RIGHT_SHIFT) )
    {
        V = 10.0f * V;
    }

    osg::Vec3 motion = forward * (V * delta_time);
    osg::Vec3 shift = traverse * (V * delta_time);

    switch (ea.getKey())
    {
    case osgGA::GUIEventAdapter::KEY_Up:
        {
            rel_pos += motion;

            break;
        }

    case osgGA::GUIEventAdapter::KEY_Down:
        {
            rel_pos -= motion;

            break;
        }

    case osgGA::GUIEventAdapter::KEY_Right:
        {
            rel_pos += shift;

            break;
        }

    case osgGA::GUIEventAdapter::KEY_Left:
        {
            rel_pos -= shift;

            break;
        }

    default:

        break;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void FreeManipulator::dragMouseProcess(const osgGA::GUIEventAdapter &ea,
                                       osgGA::GUIActionAdapter &aa)
{
    if (ea.getButtonMask() == osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON)
    {
        float k1 = osg::PIf;
        float k2 = osg::PI_2f;

        float pos_X = ea.getXnormalized();
        float pos_Y = ea.getYnormalized();

        angle_H = k1 * pos_X;
        angle_V = k2 * pos_Y;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void FreeManipulator::pushMouseProcess(const osgGA::GUIEventAdapter &ea,
                                       osgGA::GUIActionAdapter &aa)
{
    if (ea.getButton() == osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON)
    {
        float k1 = osg::PIf;
        float k2 = osg::PI_2f;

        float pos_X = ea.getXnormalized();
        float pos_Y = ea.getYnormalized();

        angle_H = k1 * pos_X;
        angle_V = k2 * pos_Y;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void FreeManipulator::releaseMouseProcess(const osgGA::GUIEventAdapter &ea,
                                          osgGA::GUIActionAdapter &aa)
{
    fixed = false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void FreeManipulator::scrollProcess(const osgGA::GUIEventAdapter &ea,
                                    osgGA::GUIActionAdapter &aa)
{
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
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void FreeManipulator::update(osg::Camera *camera)
{
    osg::Vec3 eye;
    osg::Vec3 center;
    osg::Vec3 up;

    camera->getViewMatrixAsLookAt(eye, center, up);

    osg::Vec3 dir = center - eye;
    dir.z() = 0.0;
    dir.normalize();

    forward = dir;
    traverse = dir ^ osg::Z_AXIS;
}
