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

    virtual osg::Matrixd getMatrix() const;

    virtual osg::Matrixd getInverseMatrix() const;

    virtual void init(const osgGA::GUIEventAdapter &ea,
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

    void keysDownProcess(const osgGA::GUIEventAdapter &ea,
                         osgGA::GUIActionAdapter &aa);

    void dragMouseProcess(const osgGA::GUIEventAdapter &ea,
                          osgGA::GUIActionAdapter &aa);

    virtual void pushMouseProcess(const osgGA::GUIEventAdapter &ea,
                                  osgGA::GUIActionAdapter &aa);

    virtual void releaseMouseProcess(const osgGA::GUIEventAdapter &ea,
                                     osgGA::GUIActionAdapter &aa);

    virtual void scrollProcess(const osgGA::GUIEventAdapter &ea,
                               osgGA::GUIActionAdapter &aa);

    void update(osg::Camera *camera);
};

#endif // FREE_MANIPULATOR_H
