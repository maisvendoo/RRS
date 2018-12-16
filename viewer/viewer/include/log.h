#ifndef     LOG_H
#define     LOG_H

#include    <fstream>
#include    <osg/Notify>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class LogFileHandler : public osg::NotifyHandler
{
public:

    LogFileHandler(const std::string &path);

    virtual ~LogFileHandler();

    virtual void notify(osg::NotifySeverity severity, const char *msg);

protected:

    std::ofstream   _log;
};

#endif  // LOG_H
