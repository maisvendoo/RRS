//-----------------------------------------------------------------------------
//
//      Класс TCP сервера для общения с TCP клиентами
//      (c) РГУПС, ВЖД 29/11/2017
//      Разработал: Ковшиков С. В., Притыкин Д. Е.
//
//-----------------------------------------------------------------------------
/*!
 *  \file
 *  \brief Класс TCP сервера для общения с TCP клиентами
 *  \copyright РГУПС, ВЖД
 *  \author Ковшиков С. В., Притыкин Д. Е.
 *  \date 29/11/2017
 */

#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QTcpServer>
#include <QMap>

#include "a-tcp-namespace.h"

class QTcpSocket;

class AbstractClientDelegate;
class ClientFace;
class DummyDelegate;
class AbstractEngineDefiner;


#if defined(TCPCONNECTION_LIB)
    #define TCPSERVER_EXPORT Q_DECL_EXPORT
#else
    #define TCPSERVER_EXPORT Q_DECL_IMPORT
#endif


typedef QMap<QTcpSocket*, AbstractClientDelegate*> NewList;
typedef QMap<QString, AbstractClientDelegate*> AuthList;


/*!
 * \class TcpServer
 * \brief Реализация логики работы TCP-сервера
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class TCPSERVER_EXPORT TcpServer : public QTcpServer
{
    Q_OBJECT

public:
    /// Конструктор
    TcpServer(QObject* parent = Q_NULLPTR);
    /// Деструктор
    virtual ~TcpServer();

    /// Запуск сервера
    void start(quint16 port);

    /// Установить список допустимых имён клиентов
    void setPossibleClients(QStringList names);

    /// Установить класс определения механизма подготовки данных клиентов
    void setEngineDefiner(AbstractEngineDefiner *definer);

    /// Вернуть доступную часть интерфейса клиента по имени
    ClientFace *getClient(QString clientName);

    /// Включить/отключить класс делегата-пустышки
    void enableDummy(bool enabled = true);


signals:
    /// Сигнал успешной авторизации клиента
    void clientAuthorized(ClientFace* clFace);

    /// Сигнал при отключении клиента
    void clientAboutToDisconnect(ClientFace* clFace);

    /// Печать данных в лог
    void logPrint(ATcp::ServerCodes logId, QString msg = "");


private:
    // Класс определения механизма подготовки данных клиентов
    AbstractEngineDefiner*
                engineDefiner_; ///< Опредилитель механизмов подготовки данных

    // Список допустимых имён клиентов
    QStringList guestList_; ///< Список допустимых имён клиентов

    // Список подключенных клиентов
    NewList newClients_; ///< Список подключенных клиентов

    // Список авторизованных клиентов
    AuthList authorizedClients_; ///< Список авторизованных клиентов

    // Класс-пустышка делегата клиента
    DummyDelegate* dummyClient_; ///< Класс-пустышка делегата клиента
    /*
        Класс пустышка позволяет избежать исключений из-за отсутствия клиента
        с запрашиваемым именем. При дективации имеет значение Q_NULLPTR,
        следовательно требует дополнительные проверки на Q_NULLPTR
        В духе М. Фаулера.
    */

    /// Авторизовать клиента с заданным именем
//    void authorizeClient_(AbstractClientDelegate* clnt, QByteArray name);
    void authorizeClient_(AbstractClientDelegate *clnt);

    ///
    void handleCommand_(ATcp::TcpCommand cmd, AbstractClientDelegate* clnt);


private slots:
    /// Прием соединения клиентов
    void clientConnection_();

    /// Прием данных от клиентов
    void receive_();

    /// Обработка отключения клиентов
    void clientDisconnected_();

    /// Обработка ошибок соединений
    void onAcceptError(QAbstractSocket::SocketError error);
};

#endif // TCPSERVER_H
