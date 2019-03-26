#include    "stat-manipulator.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
StaticManipulator::StaticManipulator(settings_t settings, QObject *parent)
    : AbstractManipulator(parent)
    , settings(settings)
    , init_pos(camera_position_t())
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void StaticManipulator::init(const osgGA::GUIEventAdapter &ea,
                             osgGA::GUIActionAdapter &aa)
{
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
    float dist = 10.0f;

    osg::Vec3 shift = init_pos.view_basis.right * dist;
    shift += osg::Vec3(0.0, 0.0, 2.0f);

    osg::Matrix invMatrix = osg::Matrix::lookAt(init_pos.viewer_pos + shift, cp.position, osg::Z_AXIS);

    return invMatrix;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
StaticManipulator::~StaticManipulator()
{

}
