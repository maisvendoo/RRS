#include    "abstract-manipulator.h"

AbstractManipulator::AbstractManipulator(QObject *parent)
    : QObject (parent)
    , osgGA::TrackballManipulator ()
    , cp(camera_position_t())
{
    qRegisterMetaType<camera_position_t>();
}

bool AbstractManipulator::handle(const osgGA::GUIEventAdapter &ea,
                                 osgGA::GUIActionAdapter &aa)
{
    switch (ea.getEventType())
    {
    case osgGA::GUIEventAdapter::KEYDOWN:

        keysDownProcess(ea, aa);

        break;

    case osgGA::GUIEventAdapter::DRAG:

        dragMouseProcess(ea, aa);

        break;

    case osgGA::GUIEventAdapter::RELEASE:

        releaseMouseProcess(ea, aa);

        break;
    }

    return false;
}

AbstractManipulator::~AbstractManipulator()
{

}

void AbstractManipulator::keysDownProcess(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa)
{
    Q_UNUSED(ea)
}

void AbstractManipulator::keysUpProcess(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa)
{
    Q_UNUSED(ea)
}

void AbstractManipulator::dragMouseProcess(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa)
{
    Q_UNUSED(ea)
}

void AbstractManipulator::moveMouseProcess(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa)
{
    Q_UNUSED(ea)
}

void AbstractManipulator::pushMouseProcess(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa)
{
    Q_UNUSED(ea)
}

void AbstractManipulator::releaseMouseProcess(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa)
{
    Q_UNUSED(ea)
}

void AbstractManipulator::getCameraPosition(camera_position_t cp)
{
    this->cp = cp;
}
