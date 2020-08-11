#include "JournalSyslog.h"


//--------------------------------------------------------------------
JournalSyslog::JournalSyslog( unsigned int level )
    : JournalStorage(level)
{
    openlog("", LOG_CONS | LOG_PID, LOG_USER);
}

//--------------------------------------------------------------------
JournalSyslog::~JournalSyslog()
{
    closelog();
}

//--------------------------------------------------------------------
void JournalSyslog::write(const QDateTime& time, JournalLevel::Level level, const QString& record )
{
    JournalLevel::Level logLevel = (level == JournalLevel::TrackParameters) ? JournalLevel::Trace : level;

    if (!(logLevel & JournalStorage::level())) return;

    QString message = QString("%1: [%2] %3")
            .arg(JournalLevel::printable(level))
            .arg(time.toString("yyyy-MM-dd hh:mm:ss"))
            .arg(record);

    syslog( LOG_INFO, "%s", message.toLocal8Bit().data()) ;
}
