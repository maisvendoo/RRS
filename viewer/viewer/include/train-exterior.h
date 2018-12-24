//------------------------------------------------------------------------------
//
//
//
//
//------------------------------------------------------------------------------

#ifndef     TRAINE_XTERIOR_H
#define     TRAINE_XTERIOR_H

#include    <osgGA/GUIEventHandler>
#include    <osg/MatrixTransform>

#include    "abstract-path.h"
#include    "trajectory-element.h"

#include    <osgViewer/Viewer>

#include    "vehicle-exterior.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class TrainExteriorHandler : public osgGA::GUIEventHandler
{
public:

    TrainExteriorHandler(MotionPath *routePath, const std::string &train_config);

    virtual bool handle(const osgGA::GUIEventAdapter &ea,
                        osgGA::GUIActionAdapter &aa);

    osg::Group *getExterior();

private:

    int cur_vehicle;
    float long_shift;

    osg::ref_ptr<MotionPath> routePath;

    osg::ref_ptr<osg::Group> trainExterior;

    double ref_time;
    double start_time;

    std::vector<vehicle_exterior_t> vehicles_ext;

    network_data_t  nd;

    void load(const std::string &train_config);

    void moveTrain(double ref_time, const network_data_t &nd);

    void processServerData(const network_data_t *server_data);

    void moveCamera(osgViewer::Viewer *viewer);

    void setAxis(osg::Group *vehicle,
                 osg::MatrixTransform *wheel,
                 const std::string &config_name);
};

#endif // TRAIN_EXTERIOR_H
