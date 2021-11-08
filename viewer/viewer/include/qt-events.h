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

#ifndef     QT_EVENTS_H
#define     QT_EVENTS_H

#include    <osgGA/GUIEventHandler>
#include    <QApplication>

struct TimerInfo : public osg::Referenced
{
    TimerInfo(unsigned int c) : _count(c) {}
    unsigned int _count;
};


//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class QtEventsHandler : public osgGA::GUIEventHandler
{
public:

    QtEventsHandler(unsigned int interval = 100);

    virtual bool handle(const osgGA::GUIEventAdapter &ea,
                        osgGA::GUIActionAdapter &aa);

protected:

    unsigned int _count;
    double _startTime;
    unsigned int _interval;
    unsigned int _time;
};

#endif // QT_EVENTS_H
