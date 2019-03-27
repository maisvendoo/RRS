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

    bool performMovementRightMouseButton(const double eventTimeDelta,
                                         const double dx,
                                         const double dy);

    bool handleMouseWheel(const osgGA::GUIEventAdapter &ea,
                          osgGA::GUIActionAdapter &aa);

    bool handleKeyDown(const osgGA::GUIEventAdapter &ea,
                       osgGA::GUIActionAdapter &aa);

protected:

    virtual ~TrainManipulator();

private:

    settings_t      settings;

    osg::Vec3       rel_pos;
    double          angle_H;
    double          angle_V;
};

#endif // TRAIN_MANIPULATOR_H
