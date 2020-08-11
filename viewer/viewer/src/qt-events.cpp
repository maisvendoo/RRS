//------------------------------------------------------------------------------
//
//      Qt events loop handler
//      (c) maisvendoo, 18/12/2018
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief Qt events loop handler
 * \copyright maisvendoo
 * \author maisvendoo
 * \date 18/12/2018
 */

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
            // Process qt signals and event
            QApplication::processEvents(QEventLoop::AllEvents);

            break;
        }

    default:

        break;
    }

    return false;
}
