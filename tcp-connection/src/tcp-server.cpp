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

#include "tcp-server.h"

#include <QTcpSocket>
#include <QStringBuilder>

#include "tcp-structs.h"
#include "abstract-engine-definer.h"
#include "client-delegates.h"


//------------------------------------------------------------------------------
// Конструктор
//------------------------------------------------------------------------------
TcpServer::TcpServer(QObject *parent)
    : QTcpServer(parent)
    , engineDefiner_(new NullDataEngineDefiner())
    , dummyClient_(new DummyDelegate())
{
    connect(this, SIGNAL(newConnection()), this, SLOT(clientConnection_()));

    connect(this, SIGNAL(acceptError(QAbstractSocket::SocketError)),
            this, SLOT(onAcceptError(QAbstractSocket::SocketError)));
}




//------------------------------------------------------------------------------
// Деструктор
//------------------------------------------------------------------------------
TcpServer::~TcpServer()
{
    // Удаляем опредилитель механизмов данных
    if (engineDefiner_)
        delete engineDefiner_;

    // Удаляем клиента-пустышку
    if (dummyClient_)
        delete dummyClient_;

    // Удаляем всех делегатов клиентов
    QMap<QTcpSocket*, AbstractClientDelegate*>::iterator it;

    for (it = newClients_.begin(); it != newClients_.end(); it++)
    {
        delete it.value();
    }

    newClients_.clear();
    authorizedClients_.clear();
}



//-----------------------------------------------------------------------------
// Запуск сервера
//-----------------------------------------------------------------------------
void TcpServer::start(quint16 port)
{
    if (!isListening())
    {
        if (listen(QHostAddress::Any, port))
        {
            emit logPrint(ATcp::sc_OK_SERVER_STARTED, QString::number(port));
        }
        else
        {
            emit logPrint(ATcp::sc_ER_SERVER_NOT_STARTED,
                          QString::number(port));
        }
    }
}



//-----------------------------------------------------------------------------
// Установить список допустимых имён клиентов
//-----------------------------------------------------------------------------
void TcpServer::setPossibleClients(QStringList names)
{
    guestList_ = names;
}



//-----------------------------------------------------------------------------
// Установить класс определения механизма подготовки данных клиентов
//-----------------------------------------------------------------------------
void TcpServer::setEngineDefiner(AbstractEngineDefiner *definer)
{
    if (engineDefiner_)
        delete engineDefiner_;

    engineDefiner_ = definer;
}



//-----------------------------------------------------------------------------
// Вернуть доступную часть интерфейса клиента по имени
//-----------------------------------------------------------------------------
ClientFace *TcpServer::getClient(QString clientName)
{
    return authorizedClients_.value(clientName, dummyClient_)->face();
}



//-----------------------------------------------------------------------------
// Включить/отключить класс делегата-пустышки
//-----------------------------------------------------------------------------
void TcpServer::enableDummy(bool enabled)
{
    if (enabled)
    {
        if (!dummyClient_)
            dummyClient_ = new DummyDelegate();
    }
    else
    {
        if (dummyClient_)
            delete dummyClient_;
        dummyClient_ = Q_NULLPTR;
    }
}



//-----------------------------------------------------------------------------
// Авторизовать клиента с заданным именем
//-----------------------------------------------------------------------------
void TcpServer::authorizeClient_(AbstractClientDelegate *clnt)
{
    emit logPrint(ATcp::sc_IN_AUTHORIZATION_REQUEST,
                  QString::number(clnt->getId()));

    // Предварительно запоминаем имя, присланное от клиента
    clnt->rememberName();

    // Если имя, присланное клиентом пусто
    if (clnt->getName().isEmpty())
    {
        clnt->sendAuthorizationResponse(ATcp::ar_EMPTY_NAME);

        emit logPrint(ATcp::sc_ER_CLIENT_EMPTY_NAME,
                      QString::number(clnt->getId()));

        return;
    }

    // Если список допустимых клиентов не пустой и клиент не в этом списке
    if ( !guestList_.isEmpty() && !guestList_.contains(clnt->getName()) )
    {
        clnt->sendAuthorizationResponse(ATcp::ar_UNKNOWN_NAME);

        emit logPrint(ATcp::sc_ER_CLIENT_UNKNOWN_NAME,
                      clnt->getName() % ":" % QString::number(clnt->getId()));

        clnt->forgetName();

        return;
    }
    /*
        Если список допустимых клиентов пуст, то рассматриваются все клиенты
        с уникальными именами(см. след. проверку)
    */

    // Если клиент с таким именем уже есть в списке авторизованных
    if (authorizedClients_.contains(clnt->getName()))
    {
        clnt->sendAuthorizationResponse(ATcp::ar_NAME_DUPLICATE);

        emit logPrint(ATcp::sc_ER_CLIENT_NAME_DUPLICATE,
                      clnt->getName() % ":" % QString::number(clnt->getId()));

        clnt->forgetName();

        return;
    }

    /*
        Если список допустимых клиентов не задан(пуст), если имя клиента есть
        в списке допустимых клиентов и имя клиента уникально(клиент с таким
        именеи еще не авторизован), тогда авторизуем клиента
    */
    // Позволяем определителю механизмов данных установить необходимый механизм
    engineDefiner_->setDataEngine(clnt);
    authorizedClients_.insert(clnt->getName(), clnt);

    clnt->sendAuthorizationResponse(ATcp::ar_AUTHORIZED);

    emit logPrint(ATcp::sc_OK_CLIENT_AUTHORIZED,
                  clnt->getName() % ":" % QString::number(clnt->getId()));

    emit clientAuthorized(clnt->face());
}




void TcpServer::handleCommand_(ATcp::TcpCommand cmd,
                               AbstractClientDelegate *clnt)
{
    switch (cmd)
    {
    // Нулевой запрос
    case ATcp::tcZERO:
        return;
        break;

    // Запрос авторизации
    case ATcp::tcAUTHORIZATION:
        authorizeClient_(clnt);
        break;

    // Запрос данных без сохранения буффера запроса
    case ATcp::tcGET:
        clnt->sendDataToTcpClient();
        break;

    // Сохранение буффера запроса без запроса данных
    case ATcp::tcPOST:
         clnt->storeInputData();
        break;

    // Сохранение буффера запроса и запрос данных
    case ATcp::tcPOSTGET:
        clnt->storeInputData();
        clnt->sendDataToTcpClient();
        break;

    default:
        return;
        break;
    }
}



//-----------------------------------------------------------------------------
// Прием соединения клиентов
//-----------------------------------------------------------------------------
void TcpServer::clientConnection_()
{
    QTcpSocket * sock = nextPendingConnection();

    // Создаём представителя(делегата) клиента на сервере
    ClientDelegate* client = new ClientDelegate();
    client->setSocket(sock);

    newClients_.insert(sock, client);

    connect(sock, SIGNAL(readyRead()),
            this, SLOT(receive_()));

    connect(sock, SIGNAL(disconnected()),
            this, SLOT(clientDisconnected_()));

    emit logPrint(ATcp::sc_OK_NEW_CLIENT_CONNECTED,
                  QString::number(sock->socketDescriptor()));
}



//-----------------------------------------------------------------------------
// Прием данных от клиентов
//-----------------------------------------------------------------------------
void TcpServer::receive_()
{
    // Определяем сокет передающий сокет
    QTcpSocket* sock = qobject_cast<QTcpSocket*>(sender());


    // Проверяем, достаточно ли данных пришло в сокете(поместился ли запрос)
    if (!tcp_cmd_t::validInfoSize(sock))
        return;

    // Извлекаем информационную часть структуры запроса
    tcp_cmd_t::info_t info = tcp_cmd_t::extractInfo(sock);

    // Сверяем данный размер буффера с данными оставшимися в сокете
    if(!tcp_cmd_t::validBufferSize(info.bufferSize, sock))
        return;

    // Берём представителя(делегата) клиента из списка подключённых по сокету
    AbstractClientDelegate* client = newClients_.value(sock, dummyClient_);
    /*
        Если такого клиента нет - возвращается указатель на делегата-пустышку.
        Делегата-пустышку можно отключить соответствующим методом.
        Отключение делегата-пустышки потребует дополнительных проверок
        на Q_NULLPTR в основной программе!
    */

    // Проверяем на Q_NULLPTR
    if(!client)
        return;

    handleCommand_(info.command, client);
}



//------------------------------------------------------------------------------
// Обработка отключения клиентов
//------------------------------------------------------------------------------
void TcpServer::clientDisconnected_()
{
    // Определяем отключившийся сокет
    QTcpSocket* sock = qobject_cast<QTcpSocket*>(sender());

    // Получаем указатель на делегата владеющего этим сокетом
    using ClientPtr = QScopedPointer<AbstractClientDelegate>;
    /*
        Используется умный указатель для гарантированного удаления указателя
        при выходе из метода
    */
    ClientPtr client(newClients_.value(sock, Q_NULLPTR));

    if (client.isNull())
        return;

    // Удаляем делегата из всех списков
    newClients_.remove(sock);
//    if (authorizedClients_.values().contains(client.data()))
    authorizedClients_.remove(client->getName());

    emit logPrint(ATcp::sc_OK_CLIENT_DISCONNECTED,
                  client->getName() % ":" % QString::number(client->getId()));

    emit clientAboutToDisconnect(client->face());
}




//------------------------------------------------------------------------------
// Обработка ошибок соединений
//------------------------------------------------------------------------------
void TcpServer::onAcceptError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error);

    emit logPrint(ATcp::sc_ER_SERVER_INTERNAL_ERROR, errorString());
}
