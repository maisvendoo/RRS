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

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class QtEventsHandler : public osgGA::GUIEventHandler
{
public:

    QtEventsHandler();

    virtual bool handle(const osgGA::GUIEventAdapter &ea,
                        osgGA::GUIActionAdapter &aa);

protected:


};

#endif // QT_EVENTS_H
