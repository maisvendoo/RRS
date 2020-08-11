#ifndef     STAT_MANIPULATOR_H
#define     STAT_MANIPULATOR_H

#include    "abstract-manipulator.h"
#include    "settings.h"

class StaticManipulator : public AbstractManipulator
{
public:

    StaticManipulator(settings_t settings, bool is_right = true, QObject *parent = Q_NULLPTR);

    void init(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa);

    virtual osg::Matrixd getMatrix() const;

    virtual osg::Matrixd getInverseMatrix() const;

protected:

    virtual ~StaticManipulator();

private:

    settings_t          settings;
    camera_position_t   init_pos;
    bool                is_right;
};

#endif // STAT_MANIPULATOR_H
