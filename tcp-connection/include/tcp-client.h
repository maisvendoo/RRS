//-----------------------------------------------------------------------------
//
//      Класс TCP клиента для общения с TCP сервером
//      (c) РГУПС, ВЖД 29/11/2017
//      Разработал: Ковшиков С. В., Притыкин Д. Е.
//
//-----------------------------------------------------------------------------
/*!
 *  \file
 *  \brief Класс TCP клиента для общения с TCP сервером
 *  \copyright РГУПС, ВЖД
 *  \author Ковшиков С. В., Притыкин Д. Е.
 *  \date 29/11/2017
 */

#ifndef		TCPCLIENT_H
#define 	TCPCLIENT_H

#include	<QtGlobal>
#include    <QObject>
#include    <QDataStream>

#include "tcp-client-structs.h"
#include "a-tcp-namespace.h"

class QTimer;
class QTcpSocket;

struct tcp_cmd_t;

#if defined(TCPCONNECTION_LIB)
# define TCPCLIENT_EXPORT Q_DECL_EXPORT
#else
# define TCPCLIENT_EXPORT Q_DECL_IMPORT
#endif


/*!
 *  \class TcpClient
 *  \brief Класс, обслуживающий клиентское TCP/IP-соединение
 */
//-----------------------------------------------------------------------------
//	Класс, обслуживающий клиентское TCP/IP-соединение
//-----------------------------------------------------------------------------
class TCPCLIENT_EXPORT TcpClient Q_DECL_FINAL : public QObject
{
    Q_OBJECT

public:
    /// Конструктор
    explicit TcpClient();
    /// Деструктор
    ~TcpClient();

    /// Инициализация клиента
    void init(tcp_config_t tcp_config);

    /// Проверка соединения
    bool isConnected() const;

    /// Запуск клиента
    void start();

    /// Останов клиента
    void stop();

    /// Вернуть структуру состояния клиента
    const tcp_config_t getConfig() const;

    /// Отправить только запрос
    void sendToServer(ATcp::TcpCommand comm);

    /// Отправить данные на сервер с заданной командой
    void sendToServer(ATcp::TcpCommand comm, QByteArray data);

    /// Передача данных серверу
    void sendToServer(tcp_cmd_t &cmd);

    /// Вернуть принятые данные
    QByteArray getBuffer() const;

    /// Вернуть размер буффера
    int getBufferSize() const;

    /// Отключение прокси
    void setNoProxy(bool no_proxy = true);

    /// Установить ожидаемый размер данных
    void setRecvDataSize(qint64 size);

signals:
    /// Сигнал подключения клиента к серверу
    void connectedToServer();

    /// Сигнал отключения клиента от сервера
    void disconnectedFromServer();

    /// Сигнал авторизации клиента на сервере
    void authorized();

    /// Сигнал ответа об ошибке авторизации с кодом ошибки
    void authorizationDenied(ATcp::AuthResponse errId);

    /// Сигнал приёма данных
    void dataReceived(QByteArray inData);

    /// Сигнал вывода лога с кодом
    void logPrint(ATcp::ClientCodes logId, QString msg = "");


public slots:
    /// Cоединение с сервером
    void connectToServer();

    /// Прием данных от сервера
    void receive();

    /// Обработка факта соединения с сервером
    void slotConnect();

    /// Обработка факта разрыва соединения с сервером
    void slotDisconnect();


protected:
    // Клиентский сокет
    QTcpSocket      *socket; ///< Клиентский сокет

    // Входящий поток данных
    QDataStream     in; ///< Входящий поток данных

    // Текущее состояние клиента
    tcp_state_t     tcp_state; ///< Текущее состояние клиента

    // Конфигурация клиента
    tcp_config_t    tcp_config; ///< Конфигурация клиента

    // Данные, принятые от сервера
    QByteArray      incomingData_; ///< Данные, принятые от сервера

    // Последний код авторизации
    ATcp::AuthResponse lastAuthResponse_; ///< Последний код авторизации

    // Флаг авторизации
    bool is_auth;

    // Размер данных, ожидаемых от сервера
    qint64  recvDataSize;

private:
    // Таймер попыток соединения с сервером
    QTimer* timerConnector_; ///< Таймер попыток соединения с сервером

    /// Обработать ошибку авторизации
    void handleAuthError_(ATcp::ClientCodes _cl);

    /// Отправить массив байт серверу
    void sendToServer_(QByteArray send_data);


private slots:
    /// Обработка таймера соединения
    void onTimerConnector();

};

#endif // TCPCLIENT_H
