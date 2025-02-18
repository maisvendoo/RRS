#include "abstract-manipulator.h"

#include <osgViewer/Viewer>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
AbstractManipulator::AbstractManipulator(QObject* parent)
    : QObject(parent)
    , osgGA::TrackballManipulator()
    , camera_position()
    , start_time(0.0)
    , delta_time(0.0f)
{
    qRegisterMetaType<camera_position_t>();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool AbstractManipulator::handleFrame(
    const osgGA::GUIEventAdapter& event_adapter,
    osgGA::GUIActionAdapter& action_adapter
)
{
    auto* viewer = dynamic_cast<osgViewer::Viewer*>(&action_adapter);

    if (!viewer)
    {
        return false;
    }

    double time = viewer->getFrameStamp()->getReferenceTime();
    delta_time = static_cast<float>(time - start_time);
    start_time = time;

    return false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
AbstractManipulator::~AbstractManipulator() = default;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AbstractManipulator::process_displays_lock()
{
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AbstractManipulator::getCameraPosition(camera_position_t camera_position)
{
    this->camera_position = camera_position;

    process_displays_lock();
}
