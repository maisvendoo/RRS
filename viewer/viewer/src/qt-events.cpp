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

#include    <osgViewer/Viewer>

#include    <QtCore>

extern unsigned int qGlobalPostedEventsCount();

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QtEventsHandler::QtEventsHandler(unsigned int frame_div) : osgGA::GUIEventHandler()
  , frame_div(frame_div)
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
            osgViewer::Viewer *viewer = dynamic_cast<osgViewer::Viewer *>(&aa);

            unsigned int frame = viewer->getFrameStamp()->getFrameNumber();

            if (frame % frame_div == 0)
            {
                QApplication::processEvents(QEventLoop::AllEvents, 10);
            }

            break;
        }

    default:

        break;
    }

    return false;
}
