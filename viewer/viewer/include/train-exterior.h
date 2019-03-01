//------------------------------------------------------------------------------
//
//      Loading and processing train exterior
//      (c) maisvendoo, 24/12/2018
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief Loading and processing train exterior
 * \copyright maisvendoo
 * \author maisvendoo
 * \date 24/12/2018
 */

#ifndef     TRAINE_XTERIOR_H
#define     TRAINE_XTERIOR_H

#include    <QObject>

#include    <osgGA/GUIEventHandler>
#include    <osg/MatrixTransform>

#include    "abstract-path.h"
#include    "trajectory-element.h"

#include    <osgViewer/Viewer>

#include    "vehicle-exterior.h"

/*!
 * \class
 * \brief Handler of train's exterior
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class TrainExteriorHandler : public QObject, public osgGA::GUIEventHandler
{
    Q_OBJECT

public:

    /// Constructor
    TrainExteriorHandler(MotionPath *routePath, const std::string &train_config);

    /// Handle method
    virtual bool handle(const osgGA::GUIEventAdapter &ea,
                        osgGA::GUIActionAdapter &aa);

    /// Get exterior scene group
    osg::Group *getExterior();

signals:

    void setStatusBar(QString msg);

private:

    /// Vehicle number? which is a referenced for camera
    int cur_vehicle;

    /// Shift camera along train
    float long_shift;

    /// Shift camera up/down
    float height_shift;

    /// Route path (trajectory) contener
    osg::ref_ptr<MotionPath> routePath;

    /// Train exterior scene group
    osg::ref_ptr<osg::Group> trainExterior;

    /// Time between current and previous frame drawing
    double ref_time;

    /// Frame rendering begin time
    double start_time;

    /// Info about train's vehicles exterior
    std::vector<vehicle_exterior_t> vehicles_ext;

    /// Data, received from server
    network_data_t  nd;

    /// Keyboard handler (camera control)
    void keyboardHandler(int key);

    /// Load train exterior from
    void load(const std::string &train_config);

    /// Moving train along track
    void moveTrain(double ref_time, const network_data_t &nd);

    /// Processing movind data from server
    void processServerData(const network_data_t *server_data);

    /// Move camera along track
    void moveCamera(osgViewer::Viewer *viewer);

    void recalcAttitude(size_t i);
};

#endif // TRAIN_EXTERIOR_H
