#ifndef     RAILS_MANIPULATOR_H
#define     RAILS_MANIPULATOR_H

#include    <QObject>

#include    <osgGA/TrackballManipulator>

#include    "abstract-manipulator.h"
#include    "settings.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class RailsManipulator : public AbstractManipulator
{
    Q_OBJECT

public:

    RailsManipulator(settings_t settings, QObject *parent = Q_NULLPTR);

    virtual osg::Matrixd getMatrix() const;

    virtual osg::Matrixd getInverseMatrix() const;

    bool performMovementRightMouseButton(const double eventTimeDelta,
                                         const double dx,
                                         const double dy);

    bool handleMouseWheel(const osgGA::GUIEventAdapter &ea,
                          osgGA::GUIActionAdapter &aa);

    bool handleKeyDown(const osgGA::GUIEventAdapter &ea,
                       osgGA::GUIActionAdapter &aa);

    bool handleMousePush(const osgGA::GUIEventAdapter &ea,
                         osgGA::GUIActionAdapter &aa);

protected:

    virtual ~RailsManipulator();

private:

    settings_t  settings;

    float       angle_H;
    float       angle_V;

    osg::Vec3   rel_pos;
};

#endif // RAILS_MANIPULATOR_H
