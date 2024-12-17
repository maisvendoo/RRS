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

#ifndef     TRAIN_EXTERIOR_H
#define     TRAIN_EXTERIOR_H

#include    <QObject>
#include    <QSharedMemory>

#include    <osgGA/GUIEventHandler>
#include    <osg/MatrixTransform>

#include    <osgViewer/Viewer>

#include    "vehicle-exterior.h"
#include    "animation-manager.h"
#include    "camera-position.h"
#include    "settings.h"
#include    "global-const.h"
#include    "simulator-info-struct.h"
#include    "simulator-update-struct.h"
#include    "config-reader.h"
#include    "display.h"

#include    "sound-manager.h"

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
    TrainExteriorHandler(settings_t settings, SoundManager *sm);
    ~TrainExteriorHandler();

    /// Loading vehicles
    void load(const simulator_vehicles_info_t &info_data);

    /// Handle method
    virtual bool handle(const osgGA::GUIEventAdapter &ea,
                        osgGA::GUIActionAdapter &aa);

    int getControlledVehicle();
    int getCurrentVehicle();

    /// Get exterior scene group
    osg::Group *getExterior();

    std::vector<AnimationManager *> getAnimManagers();

signals:

    //void setStatusBar(std::wstring &msg);
    void setStatusBar(QString &msg);

    void sendCameraPosition(camera_position_t cp);

    void sendControlledState(bool state);

    void sendControlledVehicle();

private:

    settings_t  settings;

    /// Sound manager
    SoundManager *sound_manager;

    /// Vehicle number which is a referenced for camera
    int cur_vehicle = 0;
    int prev_cur_vehicle = -1;

    /// Vehicle number which is contorolled by user
    int controlled_vehicle = 0;
    int prev_controlled_vehicle = -1;

    /// Train exterior scene group
    osg::ref_ptr<osg::Group> trainExterior = new osg::Group;

    /// Camera position at previous frame
    osg::Vec3f prev_camera_pos = {0.0f, 0.0f, 0.0f};

    /// Time stamp of current frame
    double ref_time = 0.0;

    /// Time stamp of previous display update
    double prev_time_display_upd = 0.0;

    ///
    bool is_displays_locked = false;

    bool is_Shift_L = false;
    bool is_Shift_R = false;
    bool is_Ctrl_L = false;
    bool is_Ctrl_R = false;
    bool is_Alt_L = false;
    bool is_Alt_R = false;

    /// Info about train's vehicles exterior
    std::vector<vehicle_exterior_t> vehicles_ext;

    enum {DATA_ARRAY_SIZE = 5};
    /// Data about vehicles positions, received from server
    std::array<simulator_update_pos_t, DATA_ARRAY_SIZE> update_pos_data;
    short new_data = -1;
    short delay_data = -1;
    short cur_data = -1;
    short old_data = -1;
    short unused_data = -1;
    double time_difference = 0.0;

    /// Debug strings for controlled and current vehicles
    simulator_vehicle_controlled_update_t vehicle_controlled;

    bool is_pos_updated = false;
    bool is_state_updated = false;

    /// Data about vehicles state, received from server
    simulator_update_t update_data;
/*
    QSharedMemory   memory_sim_update;
    QSharedMemory   memory_controlled;
*/
    /// Animations list
    std::vector<AnimationManager *> anim_managers;

    /// Moving vehicles
    void moveTrain();
/*
    /// Processing data from server
    void updatePosData(double &ref_time);
*/
    void updateDebugString();

    /// Move camera
    void moveCamera(osgViewer::Viewer *viewer, float delta_time);

    /// Load vehicle sounds
    void loadSounds(const std::string &configDir, const std::string &configName,
                    std::vector<size_t> &sounds_id);

    /// Load vehicle animations
    void loadAnimations(const std::string &configDir, const std::string &configName,
                        osg::Node *cabine, animations_t &animations);

    void loadModelAnimations(const std::string &configDir, const std::string &configName,
                             osg::Node *model, animations_t &animations);

    void loadDisplays(const std::string &configDir, osg::Node *model, displays_t &displays);

    void updateDisplays();

public slots:

    void lock_display(bool lock);

    void slotGetVehiclesPosData(QByteArray &data);

    void slotGetVehiclesStateData(QByteArray &data);

    void slotGetVehicleControlled(QByteArray &data);
};

#endif // TRAIN_EXTERIOR_H
