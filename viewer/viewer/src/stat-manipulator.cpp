#include    "stat-manipulator.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
StaticManipulator::StaticManipulator(settings_t settings, bool is_right, QObject *parent)
    : AbstractManipulator(parent)
    , settings(settings)
    , init_pos(camera_position_t())
    , is_right(is_right)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void StaticManipulator::init(const osgGA::GUIEventAdapter &ea,
                             osgGA::GUIActionAdapter &aa)
{
    Q_UNUSED(ea)
    Q_UNUSED(aa)

    init_pos = cp;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
osg::Matrixd StaticManipulator::getMatrix() const
{
    osg::Matrix matrix;

    return matrix;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
osg::Matrixd StaticManipulator::getInverseMatrix() const
{
    float dist = settings.stat_cam_dist;

    int dir = 1;

    is_right ? dir = 1 : dir = -1;

    osg::Vec3 shift = init_pos.right * dist * dir;
    shift += osg::Vec3(0.0, 0.0, settings.stat_cam_height);

    osg::Matrix invMatrix = osg::Matrix::lookAt(init_pos.viewer_pos + shift, cp.position, osg::Z_AXIS);

    return invMatrix;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
StaticManipulator::~StaticManipulator()
{

}
