#ifndef     RAILS_MANIPULATOR_H
#define     RAILS_MANIPULATOR_H

#include    <QObject>

#include    <osgGA/TrackballManipulator>
#include    <osg/Matrix>

#include    "train-position.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class RailsManipulator : public QObject, public osgGA::TrackballManipulator
{
    Q_OBJECT

public:

    RailsManipulator();

    virtual osg::Matrixd getMatrix() const;
    virtual osg::Matrixd getInverseMatrix() const;

    virtual bool handle(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa);

protected:

    virtual ~RailsManipulator();

private:

    train_position_t tp;

    osg::Vec3 position;
    osg::Vec3 attitude;
    osg::Vec3 dp;

    osg::Vec3 rel_dp;

    float pos_X0, pos_Y0;

    float angleH, angleV;

public slots:

    void getTrainPosition(train_position_t tp);
};

#endif // RAILS_MANIPULATOR_H
