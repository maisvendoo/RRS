#include    "train-manipulator.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TrainManipulator::TrainManipulator(settings_t settings, QObject *parent)
    : AbstractManipulator(parent)
    , settings(settings)
    , rel_pos(osg::Vec3f(settings.ext_cam_init_shift,
                         settings.ext_cam_init_height,
                         settings.ext_cam_init_dist))
    , angle_H(static_cast<double>(settings.ext_cam_init_angle_H))
    , angle_V(static_cast<double>(settings.ext_cam_init_angle_V))
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
osg::Matrixd TrainManipulator::getMatrix() const
{
    osg::Matrix matrix;

    float a_x = -cp.attitude.x();
    float a_z = -cp.attitude.z();

    matrix *= osg::Matrix::rotate(static_cast<double>(a_x), osg::Vec3(1.0f, 0.0f, 0.0f));
    matrix *= osg::Matrix::rotate(static_cast<double>(a_z), osg::Vec3(0.0f, 0.0f, 1.0f));
    matrix *= osg::Matrix::translate(cp.position);

    return matrix;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
osg::Matrixd TrainManipulator::getInverseMatrix() const
{
    osg::Matrix invMatrix = osg::Matrix::inverse(getMatrix());

    invMatrix *= osg::Matrix::rotate(osg::PI_2 + angle_H, osg::Vec3(0, 1, 0));
    invMatrix *= osg::Matrix::rotate(angle_V, osg::Vec3(1, 0, 0));
    invMatrix *= osg::Matrix::translate(-rel_pos);

    return invMatrix;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool TrainManipulator::performMovementRightMouseButton(const double eventTimeDelta,
                                                       const double dx,
                                                       const double dy)
{
    Q_UNUSED(eventTimeDelta)

    double k1 = static_cast<double>(settings.ext_cam_rot_coeff);

    angle_H += k1 * dx;
    angle_V += k1 * dy;

    return false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool TrainManipulator::handleMouseWheel(const osgGA::GUIEventAdapter &ea,
                                        osgGA::GUIActionAdapter &aa)
{
    Q_UNUSED(aa)

    osgGA::GUIEventAdapter::ScrollingMotion sm = ea.getScrollingMotion();

    float speed = settings.ext_cam_speed;

    if ( (ea.getModKeyMask() == osgGA::GUIEventAdapter::MODKEY_LEFT_SHIFT) ||
         (ea.getModKeyMask() == osgGA::GUIEventAdapter::MODKEY_LEFT_SHIFT) )
    {
        speed = settings.ext_cam_speed_coeff * speed;
    }

    switch (sm)
    {
    case osgGA::GUIEventAdapter::SCROLL_UP:

        if (  (ea.getModKeyMask() == osgGA::GUIEventAdapter::MODKEY_LEFT_CTRL) ||
              (ea.getModKeyMask() == osgGA::GUIEventAdapter::MODKEY_RIGHT_CTRL))
        {
            rel_pos.y() -= speed * delta_time;
        }
        else
        {
            rel_pos.z() += speed * delta_time;
        }

        break;

    case osgGA::GUIEventAdapter::SCROLL_DOWN:

        if (  (ea.getModKeyMask() == osgGA::GUIEventAdapter::MODKEY_LEFT_CTRL) ||
              (ea.getModKeyMask() == osgGA::GUIEventAdapter::MODKEY_RIGHT_CTRL))
        {
            rel_pos.y() += speed * delta_time;
        }
        else
        {
            rel_pos.z() -= speed * delta_time;
        }

        break;
    }

    if (rel_pos.z() <= settings.ext_cam_min_dist)
        rel_pos.z() = settings.ext_cam_min_dist;

    return false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TrainManipulator::~TrainManipulator()
{

}
