#include "JournalStorage.h"
#include <QTextStream>

//----------------------------------------------------
JournalStorage::JournalStorage( unsigned int level )
    : m_level( level )
{

}

//----------------------------------------------------
JournalStorage::~JournalStorage()
{

}

//----------------------------------------------------
unsigned int JournalStorage::level() const
{
    return m_level;
}


//----------------------------------------------------
void JournalStorage::write(const QDateTime& time, JournalLevel::Level level, const QString& record )
{
    JournalLevel::Level logLevel = (level == JournalLevel::TrackParameters) ? JournalLevel::Trace : level;

    // Реализация по умолчанию все бросает на консоль
    if ( !(logLevel & JournalStorage::level()) ) return;

    QTextStream out(stdout);

    QString outLine = QString("%1: [%2] %3%4")
                .arg( JournalLevel::printableColor(level) )
                .arg( time.toString("yyyy-MM-dd hh:mm:ss") )
                .arg( record )
                .arg( JournalLevel::endColor() );

    out << outLine << endl;

}
