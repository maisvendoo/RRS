#include    "rails-manipulator.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
RailsManipulator::RailsManipulator() : osgGA::TrackballManipulator ()
  , tp(train_position_t())
  , position(osg::Vec3())
  , attitude(osg::Vec3())
  , dp(osg::Vec3())
  , rel_dp(osg::Vec3())
  , pos_X0(0.0f)
  , pos_Y0(0.0f)
{
    qRegisterMetaType<train_position_t>();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
osg::Matrixd RailsManipulator::getMatrix() const
{
    osg::Matrix matrix = osg::Matrix::translate(osg::Vec3f(dp.x(), dp.z(), -dp.y()));
    matrix *= osg::Matrix::rotate(static_cast<double>(-attitude.x()), osg::Vec3(1.0f, 0.0f, 0.0f));
    matrix *= osg::Matrix::rotate(static_cast<double>(-attitude.z()), osg::Vec3(0.0f, 0.0f, 1.0f));
    matrix *= osg::Matrix::translate(position);

    return matrix;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
osg::Matrixd RailsManipulator::getInverseMatrix() const
{
    osg::Matrix matrix = osg::Matrix::translate(osg::Vec3f(dp.x(), dp.z(), -dp.y()));
    matrix *= osg::Matrix::rotate(static_cast<double>(-attitude.x()), osg::Vec3(1.0f, 0.0f, 0.0f));
    matrix *= osg::Matrix::rotate(static_cast<double>(-attitude.z()), osg::Vec3(0.0f, 0.0f, 1.0f));
    matrix *= osg::Matrix::translate(position);

    osg::Matrix invMatrix = osg::Matrix::inverse(matrix);

    invMatrix *= osg::Matrixf::rotate(angleV, osg::Y_AXIS);
    invMatrix *= osg::Matrixf::rotate(angleH, osg::X_AXIS);

    return invMatrix;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool RailsManipulator::handle(const osgGA::GUIEventAdapter &ea,
                              osgGA::GUIActionAdapter &aa)
{
    Q_UNUSED(aa)

    switch (ea.getEventType())
    {

    case osgGA::GUIEventAdapter::KEYDOWN:
    {
        switch (ea.getKey())
        {
        case osgGA::GUIEventAdapter::KEY_Left:

            rel_dp.x() -= 0.01f;

            break;

        case osgGA::GUIEventAdapter::KEY_Right:

            rel_dp.x() += 0.01f;

            break;

        case osgGA::GUIEventAdapter::KEY_Up:

            rel_dp.z() += 0.01f;

            break;

        case osgGA::GUIEventAdapter::KEY_Down:

            rel_dp.z() -= 0.01f;

            break;
        }

        break;
    }

    case osgGA::GUIEventAdapter::RELEASE:
    {
        if (ea.getButton() == osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON)
        {
            pos_X0 = ea.getX();
            pos_Y0 = ea.getY();
        }

        break;
    }

    case osgGA::GUIEventAdapter::DRAG:
    {
        if (ea.getButtonMask() == osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON)
        {
            float pos_X = ea.getX();
            float pos_Y = ea.getY();
            float k1 = 0.001f;

            angleV = k1 * (pos_X - pos_X0);
            angleH = k1 * (pos_Y - pos_Y0);
        }

        break;
    }

    default:

        break;
    }

    return false;
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
void RailsManipulator::getTrainPosition(train_position_t tp)
{
    position = tp.position;
    attitude = tp.attitude;
    dp = tp.driver_pos + rel_dp;
}
