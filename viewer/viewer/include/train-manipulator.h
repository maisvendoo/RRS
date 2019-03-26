#ifndef     TRAIN_MANIPULATOR_H
#define     TRAIN_MANIPULATOR_H

#include    "abstract-manipulator.h"
#include    "settings.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class TrainManipulator : public AbstractManipulator
{
public:

    TrainManipulator(settings_t settings, QObject *parent = Q_NULLPTR);

    virtual osg::Matrixd getMatrix() const;

    virtual osg::Matrixd getInverseMatrix() const;

protected:

    virtual ~TrainManipulator();

private:

    settings_t      settings;
};

#endif // TRAIN_MANIPULATOR_H
