#ifndef     RAILS_MANIPULATOR_H
#define     RAILS_MANIPULATOR_H

#include    <QObject>

#include    <osgGA/TrackballManipulator>

#include    "abstract-manipulator.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class RailsManipulator : public AbstractManipulator
{
    Q_OBJECT

public:

    RailsManipulator(QObject *parent = Q_NULLPTR);

    virtual osg::Matrixd getMatrix() const;

    virtual osg::Matrixd getInverseMatrix() const;

protected:

    virtual ~RailsManipulator();

private:

    float   pos_X0;
    float   pos_Y0;

    float   angle_H;
    float   angle_V;
    bool    fixed;

    osg::Vec3 rel_dp;

    void keysDownProcess(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa);

    void dragMouseProcess(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa);

    virtual void releaseMouseProcess(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa);

};

#endif // RAILS_MANIPULATOR_H
