//-----------------------------------------------------------------------------
//
//      Класс-пространство имен различных перечислителей
//      (c) РГУПС, ВЖД 30/11/2017
//      Разработал: Ковшиков С. В.
//
//-----------------------------------------------------------------------------
/*!
 *  \file
 *  \brief Класс-пространство имен различных перечислителей
 *  \copyright РГУПС, ВЖД
 *  \author Ковшиков С. В
 *  \date 30/11/2017
 */

#ifndef ATCPNAMESPACE_H
#define ATCPNAMESPACE_H

#include <QObject>


class ATcp
{
    Q_GADGET

public:

    /*!
     * \enum TcpCommand enum
     * \brief Перечислитель команд клиента серверу
     */
    enum TcpCommand
    {
        tcZERO,             ///< Ничего не делать
        tcAUTHORIZATION,    ///< Запрос авторизации
        tcGET,              ///< Отправить данные без сохранения буфера запроса
        tcPOST,             ///< Сохранить буфер запроса без отправки данных
        tcPOSTGET           ///< Сохранить буфер запроса и отправить данные
    };
    Q_ENUM(TcpCommand)


    /*!
     * \enum ServerLogs enum
     * \brief Перечислитель типов сообщений лога Tcp-сервера
     */
    enum ServerCodes
    {
        sc_UNKNOWN_CODE,             ///< Неизвестная ситуация
        // OK сообщения
        sc_OK_SERVER_STARTED,        ///< Сервер запущен
        sc_OK_NEW_CLIENT_CONNECTED,  ///< Подключен новый клиент
        sc_OK_CLIENT_AUTHORIZED,     ///< Клиент авторизован
        sc_OK_CLIENT_DISCONNECTED,   ///< Клиент отключился
        // INFO сообщения
        sc_IN_AUTHORIZATION_REQUEST, ///< Клиент запрашивает авторизацию
        // ERROR сообщения
        sc_ER_SERVER_NOT_STARTED,    ///< Сервер не запущен
        sc_ER_CLIENT_NAME_DUPLICATE, ///< Дублирование имён клиентов
        sc_ER_CLIENT_UNKNOWN_NAME,   ///< Неизвестное имя клиента
        sc_ER_CLIENT_EMPTY_NAME,     ///< Пустое имя клиента
        sc_ER_SERVER_INTERNAL_ERROR  ///< Внутренняя ошибка QTcpServer
    };
    Q_ENUM(ServerCodes)


    /*!
     * \enum ClientLogs
     * \brief Перечислитель типов сообщений лога Tcp-клиента
     */
    enum ClientCodes
    {
        cc_UNKNOWN_CODE,        ///< Неизвестная ситуация
        //
        cc_IN_AUTHORIZATION,    ///< Запрос авторизации
        // OK сообщения
        cc_OK_CONNECTED,
        cc_OK_AUTHOROZED,       ///< Клиент авторизован
        // ERROR сообщения
        cc_ER_NAME_DUPLICATE,   ///< Дублирование имён
        cc_ER_UNKNOWN_NAME,     ///< Неизвестное имя клиента
        cc_ER_EMPTY_NAME        ///< Пустое имя
    };
    Q_ENUM(ClientCodes)


    /*!
     * \enum AuthResponse
     * \brief Перечислитель кодов авторизации
     */
    enum AuthResponse
    {
        ar_NO_RESONSE,      ///< Нулевое значение
        ar_AUTHORIZED,      ///< Авторизован
        ar_NAME_DUPLICATE,  ///< Отказ авторизации, дублирование имён
        ar_UNKNOWN_NAME,    ///< Отказ авторизации, неизвестное имя
        ar_EMPTY_NAME       ///< Отказ авторизации, пустое имя
    };
    Q_ENUM(AuthResponse)


    /// Преобразовать структуру в QByteArray (структура должна быть копируемой)
    template<typename T>
    static QByteArray toByteArray(T data, bool *ok = Q_NULLPTR)
    {
        // Проверяем возможность копирования структуры
        bool tempOk = std::is_trivially_copyable<T>::value;
        if (ok)
            *ok = tempOk;

        // Если структура mem-копируема
        if (tempOk)
        {
            // Генерируем массив
            QByteArray arr(sizeof(T), Qt::Uninitialized);
            memcpy(arr.data(), &data, sizeof(T));
            return arr;
        }
        // Иначе возвращаем пустой массив
        return QByteArray();
    }


    /// Преобразовать структуру в QByteArray (структура должна быть копируемой)
    template<typename T>
    static QByteArray toByteArrayHard(T data)
    {
        // Генерируем массив
        QByteArray arr(sizeof(T), Qt::Uninitialized);
        memcpy(arr.data(), &data, sizeof(T));
        return arr;
    }

};

#endif // ATCPNAMESPACE_H
