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
{
    qRegisterMetaType<camera_position_t>();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool AbstractManipulator::handle(const osgGA::GUIEventAdapter &ea,
                                 osgGA::GUIActionAdapter &aa)
{
    switch (ea.getEventType())
    {
    case osgGA::GUIEventAdapter::FRAME:
        {
            osgViewer::Viewer *viewer = dynamic_cast<osgViewer::Viewer *>(&aa);

            if (!viewer)
                break;

            double time = viewer->getFrameStamp()->getReferenceTime();
            delta_time = static_cast<float>(time - start_time);
            start_time = time;

            break;
        }

    case osgGA::GUIEventAdapter::KEYDOWN:

        keysDownProcess(ea, aa);

        break;

    case osgGA::GUIEventAdapter::DRAG:

        dragMouseProcess(ea, aa);

        break;

    case osgGA::GUIEventAdapter::PUSH:

        pushMouseProcess(ea, aa);
        break;

    case osgGA::GUIEventAdapter::RELEASE:

        releaseMouseProcess(ea, aa);

        break;

    case osgGA::GUIEventAdapter::SCROLL:

        scrollProcess(ea, aa);

        break;

    default:

        break;
    }

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
void AbstractManipulator::keysDownProcess(const osgGA::GUIEventAdapter &ea,
                                          osgGA::GUIActionAdapter &aa)
{
    Q_UNUSED(ea)
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AbstractManipulator::keysUpProcess(const osgGA::GUIEventAdapter &ea,
                                        osgGA::GUIActionAdapter &aa)
{
    Q_UNUSED(ea)
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AbstractManipulator::dragMouseProcess(const osgGA::GUIEventAdapter &ea,
                                           osgGA::GUIActionAdapter &aa)
{
    Q_UNUSED(ea)
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AbstractManipulator::moveMouseProcess(const osgGA::GUIEventAdapter &ea,
                                           osgGA::GUIActionAdapter &aa)
{
    Q_UNUSED(ea)
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AbstractManipulator::pushMouseProcess(const osgGA::GUIEventAdapter &ea,
                                           osgGA::GUIActionAdapter &aa)
{
    Q_UNUSED(ea)
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AbstractManipulator::releaseMouseProcess(const osgGA::GUIEventAdapter &ea,
                                              osgGA::GUIActionAdapter &aa)
{
    Q_UNUSED(ea)
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AbstractManipulator::scrollProcess(const osgGA::GUIEventAdapter &ea,
                                        osgGA::GUIActionAdapter &aa)
{

}

void AbstractManipulator::getCameraPosition(camera_position_t cp)
{
    this->cp = cp;
}
