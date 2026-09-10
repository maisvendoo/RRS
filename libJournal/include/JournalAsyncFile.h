#ifndef JOURNALASYNCFILE_H_
#define JOURNALASYNCFILE_H_

#include "JournalStorage.h"

#include <QFile>
#include <QString>

#include <atomic>
#include <condition_variable>
#include <deque>
#include <thread>

/**
 * @brief Асинхронное журнальное хранилище: очередь сообщений
 *        и фоновый поток записи (низший приоритет), ТЗ "RP-сервер" п.4, 7
 */
class JournalAsyncFile
    : public JournalStorage
{

public:

    /// @param fileName - путь к файлу журнала
    /// @param level - маска уровней хранилища
    JournalAsyncFile(QString fileName, unsigned int level);

    virtual ~JournalAsyncFile() override;

    /// Ставит сообщение в очередь (потокобезопасно, без блокировки диска)
    virtual void write(const QDateTime& time,
                       JournalLevel::Level level,
                       const QString& record) override;

private:

    void writeImpl(const QDateTime& time,
                   JournalLevel::Level level,
                   const QString& record);

    QFile m_file;
    std::mutex m_queue_mutex;

    struct Record
    {
        QDateTime time;
        JournalLevel::Level level = JournalLevel::None;
        QString text;
    };

    std::deque<Record> m_queue;
    std::condition_variable m_cv;
    std::thread m_worker;
    std::atomic<bool> m_stop{false};
};

#endif // JOURNALASYNCFILE_H_
