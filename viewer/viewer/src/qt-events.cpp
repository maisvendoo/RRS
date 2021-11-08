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
QtEventsHandler::QtEventsHandler(unsigned int interval) : osgGA::GUIEventHandler()
  , _count(0)
  , _startTime(0.0)
  , _interval(interval)
  , _time(0)
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

            osgViewer::Viewer *viewer = dynamic_cast<osgViewer::Viewer *>(&aa);

            if (!viewer)
                break;

            double time = viewer->getFrameStamp()->getReferenceTime();
            unsigned int delta = static_cast<unsigned int>( (time - _startTime) * 1000.0);
            _startTime = time;

            if ( (_count >= _interval) || (_time == 0) )
            {
                viewer->getEventQueue()->userEvent(new TimerInfo(_time));
                _count = 0;
            }

            _count += delta;
            _time += delta;

            break;
        }

    case osgGA::GUIEventAdapter::USER:
        {
            QApplication::processEvents(QEventLoop::AllEvents, 10);
            break;
        }

    default:

        break;
    }

    return false;
}
