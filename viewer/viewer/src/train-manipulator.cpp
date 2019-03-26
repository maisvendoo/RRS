#include    "train-manipulator.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TrainManipulator::TrainManipulator(settings_t settings, QObject *parent)
    : AbstractManipulator(parent)
    , settings(settings)
    , rel_pos(osg::Vec3d(0.0, 3.0, 25.0))
    , angle_H(0.0)
    , angle_V(0.0)
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
    double k1 = 1.0;

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
    osgGA::GUIEventAdapter::ScrollingMotion sm = ea.getScrollingMotion();

    float speed = 5.0f;

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

    if (rel_pos.z() <= 5.0f)
        rel_pos.z() = 5.0f;

    return false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TrainManipulator::~TrainManipulator()
{

}
