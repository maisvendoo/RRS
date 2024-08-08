//-----------------------------------------------------------------------------
//
//      Классы представителей(делегатов) клиентов на сервере
//      (c) РГУПС, ВЖД 30/11/2017
//      Разработал: Ковшиков С. В.
//
//-----------------------------------------------------------------------------
/*!
 *  \file
 *  \brief Классы представителей(делегатов) клиентов на сервере
 *  \copyright РГУПС, ВЖД
 *  \author Ковшиков С. В
 *  \date 30/11/2017
 */

#ifndef CLIENT_DELEGATES_H
#define CLIENT_DELEGATES_H

#include <QObject>
#include <QString>
#include <a-tcp-namespace.h>

class QTcpSocket;
class AbstractDataEngine;

class AbstractClientDelegate;


#if defined(TCPCONNECTION_LIB)
    #define DELEGATE_EX Q_DECL_EXPORT
#else
    #define DELEGATE_EX Q_DECL_IMPORT
#endif


/*!
 * \class ClientFace
 * \brief Класс предоставляющий только необходимую часть интерфейса делегата
 */
class DELEGATE_EX ClientFace Q_DECL_FINAL : public QObject
{
    Q_OBJECT

public:
    ///
    ClientFace(AbstractClientDelegate* delegate, QObject* parent = Q_NULLPTR);

    ///
    QString getName() const;

    ///
    quintptr getId() const;

    ///
    QByteArray getInputBuffer() const;

    ///
    void setOutputBuffer(QByteArray arr);


signals:
    ///
    void dataReceived(QByteArray data);


private:
    //
    AbstractClientDelegate* delegate_;
};


/*!
 * \class AbstractClientDelegate
 * \brief Базовый класс представителя(делегата) клиента на сервере
 */
class DELEGATE_EX AbstractClientDelegate : public QObject
{
    Q_OBJECT

public:
    /// Конструктор
    AbstractClientDelegate(QObject* parent = Q_NULLPTR);
    /// Деструктор
    virtual ~AbstractClientDelegate();

    /// Вернуть указатель на доступную часть интерфейса
    ClientFace *face();

    /// Вернуть имя
    QString getName() const;

    /// Установить имя
    virtual void rememberName();

    void forgetName();

    /// Установить сокет
    virtual void setSocket(QTcpSocket* sock);

    /// Вернуть дескриптор сокета
    qintptr getId() const;

    /// Установить механизм подготовки данных
    virtual void setDataEngine(AbstractDataEngine* engine);

    /// Сохранить буффер запроса от клиента
    virtual void storeInputData() = 0;

    /// Установить буффер данных для отправки клиенту
    virtual void setOutputBuffer(QByteArray buf) = 0;

    /// Вернуть буффер данных принятых от клиента
    QByteArray getInputBuffer() const;

    /// Отправить результат авторизации
    virtual void sendAuthorizationResponse(ATcp::AuthResponse resp) = 0;

    /// Отправить данные клиенту
    virtual void sendDataToTcpClient() = 0;


signals:
    /// Оповещение о приёме данных от клиента
    void dataReceived(QByteArray arr);


protected:
    //
    ClientFace* face_;
    // Имя делегата
    QString name_; ///< Имя делегата
    // Дескриптор сокета
    qintptr localId_; ///< Дескриптор сокета
    // Сокет
    QTcpSocket* socket_; ///< Сокет
    // Механизм подготовки данных
    AbstractDataEngine* engine_; ///< Механизм подготовки данных

};


/*!
 * \class DummyDelegate
 * \brief Класс "затычка"(null-object) представителя(делегата) для перенаправки
 * данных ему, в случае отсутствия клиента с необходимым именем
 */
class DELEGATE_EX DummyDelegate Q_DECL_FINAL : public AbstractClientDelegate
{
    Q_OBJECT

public:
    /// Конструктор
    DummyDelegate(QObject *parent = Q_NULLPTR);
    /// Деструктор
    ~DummyDelegate();

    /// Установить имя (пустышка)
//    void setName(QString name) Q_DECL_OVERRIDE;
    void rememberName() Q_DECL_OVERRIDE;

    /// Установить сокет (пустышка)
    void setSocket(QTcpSocket* sock) Q_DECL_OVERRIDE;

    /// Установить механизм подготовки данных (пустышка)
    void setDataEngine(AbstractDataEngine* engine) Q_DECL_OVERRIDE;

    /// Сохранить буффер запроса от клиента (пустышка)
    void storeInputData() Q_DECL_OVERRIDE;

    /// Установить буффер данных для отправки клиенту (пустышка)
    void setOutputBuffer(QByteArray buf) Q_DECL_OVERRIDE;

    /// Отправить результат авторизации (пустышка)
    void sendAuthorizationResponse(ATcp::AuthResponse resp) Q_DECL_OVERRIDE;

    /// Отправить данные клиенту (пустышка)
    void sendDataToTcpClient() Q_DECL_OVERRIDE;

};


/*!
 * \class The ClientDelegate class
 * \brief Класс представителя(делегата) клиента на сервере
 */
class DELEGATE_EX ClientDelegate Q_DECL_FINAL : public AbstractClientDelegate
{
public:
    /// Конструктор
    ClientDelegate(QObject* parent = Q_NULLPTR);
    /// Деструктор
    ~ClientDelegate();

    /// Сохранить буффер запроса от клиента
    void storeInputData() Q_DECL_OVERRIDE;

    /// Установить буффер данных для отправки клиенту
    void setOutputBuffer(QByteArray buf) Q_DECL_OVERRIDE;

    /// Отправить результат авторизации
    void sendAuthorizationResponse(ATcp::AuthResponse resp) Q_DECL_OVERRIDE;

    /// Отправить данные клиенту
    void sendDataToTcpClient() Q_DECL_OVERRIDE;

};

#endif // CLIENT_DELEGATES_H
