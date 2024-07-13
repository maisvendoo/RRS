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
#include    "simulator-info-struct.h"
#include    "simulator-update-struct.h"
#include    "controlled-struct.h"
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
    TrainExteriorHandler(settings_t settings, SoundManager *sm, const simulator_info_t &info_data);
    ~TrainExteriorHandler();

    /// Handle method
    virtual bool handle(const osgGA::GUIEventAdapter &ea,
                        osgGA::GUIActionAdapter &aa);

    /// Get exterior scene group
    osg::Group *getExterior();

    std::vector<AnimationManager *> getAnimManagers();

signals:

    void setStatusBar(std::wstring &msg);

    void sendCameraPosition(camera_position_t cp);

private:

    settings_t  settings;

    /// Vehicle number which is a referenced for camera
    int cur_vehicle;

    /// Vehicle number which is contorolled by user
    int controlled_vehicle;

    /// Shift camera along train
    float long_shift;

    /// Shift camera up/down
    float height_shift;

    /// Train exterior scene group
    osg::ref_ptr<osg::Group> trainExterior;

    /// Camera position at previous frame
    osg::Vec3f prev_camera_pos;

    /// Time stamp of previous frame
    double prev_time;

    /// Time between current and previous frame drawing
    double ref_time;

    ///
    bool is_displays_locked;

    /// Info about train's vehicles exterior
    std::vector<vehicle_exterior_t> vehicles_ext;

    /// Data, received from server
    std::array<simulator_update_t, 2> update_data;
    short new_data;
    short old_data;

    QSharedMemory   memory_sim_update;
    QSharedMemory   memory_controlled;

    /// Animations list
    std::vector<AnimationManager *> anim_managers;

    /// Sound manager
    SoundManager *sound_manager;

    /// Keyboard handler (camera control)
    void keyboardHandler(int key);

    /// Load train exterior from
    void load(const simulator_info_t &info_data);

    /// Moving train along track
    void moveTrain(double ref_time, const std::array<simulator_update_t, 2> update_data);

    /// Processing data from server
    void processSharedData(double &ref_time);

    /// Processing data from server
    void sendControlledVehicle(const controlled_t &data);

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

    void timerEvent(QTimerEvent *);

public slots:

    void lock_display(bool lock);
};

#endif // TRAIN_EXTERIOR_H
