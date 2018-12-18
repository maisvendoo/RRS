#include    "qt-events.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QtEventsHandler::QtEventsHandler()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool QtEventsHandler::handle(const osgGA::GUIEventAdapter &ea,
                             osgGA::GUIActionAdapter &aa)
{
    Q_UNUSED(aa)

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
