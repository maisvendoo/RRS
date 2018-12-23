//------------------------------------------------------------------------------
//
//      Camera's events hadnler (to process camera motion)
//      (c) maisvendoo, 01/12/2018
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief Camera's events hadnler (to process camera motion)
 * \copyright maisvendoo
 * \author maisvendoo
 * \date 01/12/2018
 */

#ifndef     CAMERA_H
#define     CAMERA_H

#include    <osgGA/GUIEventAdapter>
#include    <osgGA/GUIActionAdapter>
#include    <osgViewer/Viewer>

#include    "trajectory.h"

/*!
 * \class
 * \brief Camera pseudo-manipulator
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class RailwayManipulator : public osgGA::GUIEventHandler
{
public:

    /// Conastructor
    RailwayManipulator(Trajectory *train_traj);

    /// Events handler
    virtual bool handle(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa);

protected:

    /// Train trajectory object
    osg::ref_ptr<Trajectory>        train_traj;
    /// Reference time count between position data update
    double                          ref_time;
    /// Initial time of frame draw
    double                          _startTime;
    /// Current trajectory element
    network_data_t                  traj_element;
    /// Camera's view matrix
    osg::Matrix                     matrix;

    /// Camera motion
    void moveCamera(double ref_time,
                    const network_data_t &traj_element,
                    osgViewer::Viewer *viewer);

    /// Set data to processed trajectory element
    void setTrajectoryElement(const network_data_t *te);
};

#endif // CAMERA_H
