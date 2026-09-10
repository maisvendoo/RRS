#include "JournalAsyncFile.h"

#include <QDir>
#include <QFileInfo>
#include <QTextStream>

//--------------------------------------------------------------------
JournalAsyncFile::JournalAsyncFile(QString fileName, unsigned int level)
    : JournalStorage(level)
    , m_file(fileName)
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

    m_worker = std::thread([this]()
    {
        while (true)
        {
            Record record;

            {
                std::unique_lock<std::mutex> lock(m_queue_mutex);
                m_cv.wait(lock, [this]()
                {
                    return m_stop.load() || !m_queue.empty();
                });

                if (m_stop.load() && m_queue.empty())
                {
                    break;
                }

                record = std::move(m_queue.front());
                m_queue.pop_front();
            }

            writeImpl(record.time, record.level, record.text);
        }
    });
}

//--------------------------------------------------------------------
JournalAsyncFile::~JournalAsyncFile()
{
    m_stop.store(true);
    {
        std::lock_guard<std::mutex> lock(m_queue_mutex);
        m_cv.notify_all();
    }

    if (m_worker.joinable())
    {
        m_worker.join();
    }

    m_file.close();
}

//--------------------------------------------------------------------
void JournalAsyncFile::write(const QDateTime& time,
                             JournalLevel::Level level,
                             const QString& record)
{
    JournalLevel::Level logLevel =
            (level == JournalLevel::TrackParameters) ? JournalLevel::Trace : level;

    if (!(logLevel & JournalStorage::level()))
        return;

    {
        std::lock_guard<std::mutex> lock(m_queue_mutex);
        m_queue.push_back(Record{time, level, record});
    }

    m_cv.notify_all();
}

//--------------------------------------------------------------------
void JournalAsyncFile::writeImpl(const QDateTime& time,
                                 JournalLevel::Level level,
                                 const QString& record)
{
    if (!m_file.isOpen())
        return;

    QTextStream out(&m_file);

    QString fileLine = QString("%1: [%2] %3")
            .arg(JournalLevel::printable(level))
            .arg(time.toString("yyyy-MM-dd hh:mm:ss"))
            .arg(record);

    out << fileLine << Qt::endl;
    out.flush();
}
