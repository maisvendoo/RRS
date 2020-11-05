#ifndef     ABSTRACT_TRAJECTORY_H
#define     ABSTRACT_TRAJECTORY_H

#include    <osg/Referenced>
#include    <osg/Geometry>
#include    "trajectory-element.h"
#include    "abstract-path.h"

#include    "import-export.h"

#ifdef ROUTE_LOADER_LIB
    #define ROUTE_LOADER_EXPORT DECL_EXPORT
#else
    #define ROUTE_LOADER_EXPORT DECL_IMPORT
#endif

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class ROUTE_LOADER_EXPORT Trajectory : public osg::Referenced
{
public:

    Trajectory();

    virtual void update(const network_data_t &traj_elem,
                        const float &delta_time) = 0;

    virtual osg::Vec3 getPosition() const = 0;

    virtual osg::Vec3 getAttitude() const = 0;

    virtual float getCurrentCoord() const = 0;

    MotionPath *getMotionPath();

protected:

    /// Route path (tracks data processing)
    osg::ref_ptr<MotionPath>    routePath;
};

#endif // ABSTRACT_TRAJECTORY_H
