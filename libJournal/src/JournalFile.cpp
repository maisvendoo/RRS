#include "JournalFile.h"
#include <QDir>
#include <QTextStream>

//--------------------------------------------------------------------
JournalFile::JournalFile(QString fileName, unsigned int level )
    : JournalStorage(level)
    , m_file(fileName)
    , m_fileMutex()
{
    if (m_file.exists())
    {
        QFileInfo fi(m_file);
        QString dir = fi.absolutePath() + QDir::separator();
        QString file = fi.fileName();

        QString backup_prefix = "~previous-";
        QString log_backup = dir + backup_prefix + file;

        QFile tmp;
        tmp.remove(log_backup);
        tmp.rename(fileName, log_backup);
    }

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
