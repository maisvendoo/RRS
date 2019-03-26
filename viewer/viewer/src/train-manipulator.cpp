#include    "train-manipulator.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TrainManipulator::TrainManipulator(settings_t settings, QObject *parent)
    : AbstractManipulator(parent)
    , settings(settings)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
osg::Matrixd TrainManipulator::getMatrix() const
{
    osg::Matrix matrix;

    return matrix;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
osg::Matrixd TrainManipulator::getInverseMatrix() const
{
    osg::Matrix invMatrix = osg::Matrix::inverse(getMatrix());

    return invMatrix;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TrainManipulator::~TrainManipulator()
{

}
