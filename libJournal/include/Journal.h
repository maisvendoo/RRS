#ifndef JOURNAL_H
#define JOURNAL_H

#include "JournalStorage.h"
#include <QHash>
#include <QMutex>
#include <QVector>

class Journal
{
protected:
    Journal(const quint64 index);

public:
    virtual ~Journal();

    static Journal*     instance();
    static Journal*     instance(const quint64 index);

    static JournalLevels journalLevels();
    static void          setJournalLevel(const JournalLevels journalLevel);

    /**
     * @brief Процедура добавляет хранилище к списку хранилищ журнала
     *
     * @param storage указатель на хранилище
     */
    void addStorage(JournalStorage* storage);

protected:
    /**
     * @brief Процедура посылает сообщение с заданным уровнем всем хранилищам, связанным с журналом
     *
     * @param level уровень сообщения
     * @param record ссылка на строку записи
     */
    void write(JournalLevel::Level level, const QString& record);

public:
    /**
     * @brief Процедура добавляет сообщение о критической ошибке в журнал
     *
     * @param record ссылка на строку сообщения
     */
    void critical(const QString& record)
    {
        write(JournalLevel::Critical, record);
    }

    /**
     * @brief Процедура добавляет сообщение об ошибке в журнал
     *
     * @param record ссылка на строку сообщения
     */
    void error(const QString& record)
    {
        write(JournalLevel::Error, record);
    }

    /**
     * @brief Процедура добавляет сообщение-предупреждение в журнал
     *
     * @param record ссылка на строку сообщения
     */
    void warning(const QString& record)
    {
        write(JournalLevel::Warning, record);
    }

    /**
     * @brief Процедура добавляет сообщение в журнал
     *
     * @param record ссылка на строку сообщения
     */
    void message(const QString& record)
    {
        write(JournalLevel::Message, record);
    }

    /**
     * @brief Процедура добавляет информационное сообщение в журнал
     *
     * @param record ссылка на строку сообщения
     */
    void info(const QString& record)
    {
        write(JournalLevel::Info, record);
    }

    /**
      * @brief Процедура добавляет отладочное сообщение в журнал
      *
      * @param record ссылка на строку сообщения
      */
    void debug(const QString& record)
    {
        write(JournalLevel::Debug, record);
    }

    /**
      * @brief Процедура добавляет подробное отладочное сообщение в журнал
      *
      * @param record ссылка на строку сообщения
      */
    void trace(const QString& record)
    {
        write(JournalLevel::Trace, record);
    }

    /**
      * @brief Процедура добавляет подробное отладочное сообщение в журнал
      *
      * @param record ссылка на строку сообщения
      */
    void traceCalls(const QString& record)
    {
        write(JournalLevel::TraceCalls, record);
    }

    /**
      * @brief Процедура добавляет подробное отладочное сообщение в журнал
      *
      * @param record ссылка на строку сообщения
      */
    void trackRuntime(const QString& record)
    {
        write(JournalLevel::TrackRuntime, record);
    }

    /**
      * @brief Сообщение с значением параметра
      * @param record ссылка на строку сообщения
      */
    void trackParameter(const QString& record)
    {
        write(JournalLevel::TrackParameters, record);
    }
private:

    /**
     * @brief Запрещен конструктор копирования
     */
    Journal(const Journal&) = delete;

    /**
     * @brief Запрещен оператор присваивания
     */
    void operator = (Journal const&) = delete;

    static QHash<quint64, Journal*>                m_instances;     ///< Список экземпляров объектов
    QVector<JournalStorage*>                       m_storages;      ///< Объект хранилища
    const quint64                                  m_index;         ///< Индекс хранилища
    static QMutex                                  m_instanceMutex;
    static JournalLevels                           m_journalLevel;
};

#endif // JOURNAL_H
