#include    "qt-events.h"

bool QtEventsHandler::handle(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa)
{
    switch (ea.getEventType())
    {
    case osgGA::GUIEventAdapter::FRAME:
        {
            QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

            break;
        }

    default:

        break;
    }

    return false;
}

QtEventsHandler::QtEventsHandler()
{

}
