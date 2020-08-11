#ifndef JOURNALSYSLOG_H_
#define JOURNALSYSLOG_H_

#include "JournalStorage.h"


#if __unix__
    #include <syslog.h>
#else
    #include "wsyslog.h"
#endif

/**
 * @brief Журнальное хранилище, связанное с системным журнальным средством обслуживания
 */
class JournalSyslog
    : public JournalStorage
{
public:
    /**
     * @brief Конструктор
     *
     * @param level маска уровней хранилища
     *
     * Открывает системное журнальное средство обслуживания для сообщений процесса со следующими флагами:
     * указывать ID процесса в каждом сообщении, при ошибках сообщать на консоль
     */
    JournalSyslog( unsigned int level );

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
    virtual ~JournalSyslog();
};

#endif /* JOURNALSYSLOG_H_ */
