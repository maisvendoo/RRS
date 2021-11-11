#include    "abstract-manipulator.h"
#include    <osgViewer/Viewer>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
AbstractManipulator::AbstractManipulator(QObject *parent)
    : QObject (parent)
    , osgGA::TrackballManipulator ()
    , cp(camera_position_t())
    , start_time(0.0)
    , delta_time(0.0f)
{
    qRegisterMetaType<camera_position_t>();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool AbstractManipulator::handleFrame(const osgGA::GUIEventAdapter &ea,
                                      osgGA::GUIActionAdapter &aa)
{
    (void) ea;

    osgViewer::Viewer *viewer = dynamic_cast<osgViewer::Viewer *>(&aa);

    if (!viewer)
        return false;

    double time = viewer->getFrameStamp()->getReferenceTime();
    delta_time = static_cast<float>(time - start_time);
    start_time = time;

    return false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
AbstractManipulator::~AbstractManipulator()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AbstractManipulator::process_displays_lock()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AbstractManipulator::getCameraPosition(camera_position_t cp)
{
    this->cp = cp;

    process_displays_lock();
}
