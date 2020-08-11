#ifndef JOURNALFILE_H_
#define JOURNALFILE_H_

#include "JournalStorage.h"
#include <QString>
#include <QFile>
#include <QMutex>


/**
 * @brief Журнальное хранилище, связанное с системным журнальным средством обслуживания
 */
class JournalFile
    : public JournalStorage
{

private:
    QFile m_file;
    QMutex m_fileMutex;


public:
    /**
     * @brief Конструктор
     *
     * @param level маска уровней хранилища
     *
     * Открывает системное журнальное средство обслуживания для сообщений процесса со следующими флагами:
     * указывать ID процесса в каждом сообщении, при ошибках сообщать на консоль
     */
    JournalFile( QString fileName, unsigned int level );

    /**
     * @brief Процедура звписывает сообщение с временем и маркером уровня в лог
     *
     * @param time время, указываемое при записи сообщения
     * @param level уровень сообщения
     * @param record C++-строка сообщения
     *
     * Если маска хранилища не имеет совпадений с уровнем сообщения, возвращается, иначе -записывает
     * сообщение с указанным временем и маркером уровня в лог с приоритетом информационного сообщения
     */
    void write( const QDateTime& time, JournalLevel::Level level, const QString& record );

    /**
     * @brief Деструктор
     */
    virtual ~JournalFile();
};


#endif /* JOURNALFILE_H_ */
