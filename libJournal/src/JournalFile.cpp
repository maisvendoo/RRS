#include "JournalFile.h"
#include <QTextStream>

//--------------------------------------------------------------------
JournalFile::JournalFile(QString fileName, unsigned int level )
    : JournalStorage(level)
    , m_file(fileName)
    , m_fileMutex()
{
    m_file.open(QIODevice::Append);
}

//--------------------------------------------------------------------
JournalFile::~JournalFile()
{
    m_file.close();
}

//--------------------------------------------------------------------
void JournalFile::write(const QDateTime& time, JournalLevel::Level level, const QString& record )
{
    QMutexLocker lock(&m_fileMutex);

    if (!m_file.isOpen())
        return;

    QTextStream out(&m_file);

    JournalLevel::Level logLevel = (level == JournalLevel::TrackParameters) ? JournalLevel::Trace : level;

    if (!(logLevel & JournalStorage::level()))
        return;

    QString fileLine = QString("%1: [%2] %3")
            .arg(JournalLevel::printable(level))
            .arg(time.toString("yyyy-MM-dd hh:mm:ss"))
            .arg(record);

    out << fileLine << Qt::endl;
    out.flush();
}
