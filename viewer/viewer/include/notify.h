//------------------------------------------------------------------------------
//
//      OSG log messages handler
//      (c) maisvendoo, 12/12/2018
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief OSG log messages handler
 * \copyright maisvendoo
 * \author maisvendoo
 * \date 12/12/2018
 */

#ifndef     NOTIFY_OSG_VIEWER_H
#define     NOTIFY_OSG_VIEWER_H

#include    <osg/Notify>
#include    <fstream>

/*!
 * \class
 * \brief Log messages handler
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class ViewerLogFileHandler : public osg::NotifyHandler
{
public:

    /// Constructor
    ViewerLogFileHandler(const std::string &file);

    /// Destructor
    virtual ~ViewerLogFileHandler();

    /// Log message handler
    virtual void notify(osg::NotifySeverity severity, const char *msg);

protected:

    /// Log output stream
    std::ofstream log;
};

#endif // NOTIFY_OSG_VIEWER_H
