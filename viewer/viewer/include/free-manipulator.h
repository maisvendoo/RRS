#ifndef     FREE_MANIPULATOR_H
#define     FREE_MANIPULATOR_H

#include    "abstract-manipulator.h"
#include    "settings.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class FreeManipulator : public AbstractManipulator
{
public:

    FreeManipulator(settings_t settings, QObject *parent = Q_NULLPTR);

    osg::Matrixd getMatrix() const;

    osg::Matrixd getInverseMatrix() const;

    void init(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa);

    bool performMovementRightMouseButton (const double eventTimeDelta,
                                          const double dx,
                                          const double dy);

    bool handleKeyDown(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa);

    bool handleMouseWheel(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa);

    bool handleMousePush(const osgGA::GUIEventAdapter &ea,
                         osgGA::GUIActionAdapter &aa);


protected:

    virtual ~FreeManipulator();

private:

    settings_t          settings;
    camera_position_t   init_pos;
    osg::Vec3           rel_pos;

    float               angle_H;
    float               angle_V;

    float               pos_X0;
    float               pos_Y0;

    bool                fixed;

    osg::Vec3f          forward;
    osg::Vec3f          traverse;

    osg::Camera         *camera;    
};

#endif // FREE_MANIPULATOR_H
