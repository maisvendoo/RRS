//------------------------------------------------------------------------------
//
//      Class for client description in server side
//      (c) maisvendoo, 18/09/2018
//      Developer: Dmitry Pritykin
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief Class for client description in server side
 * \copyright maisvendoo
 * \author Dmitry Pritykin
 * \date 18/09/2018
 */

#ifndef     CLIENT_H
#define     CLIENT_H

#include    <QObject>
#include    <QTcpSocket>

/*!
 * \class
 * \brief Client description in server's list
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class Client : public QObject
{
    Q_OBJECT

public:

    /// Constructor
    explicit Client(QObject *parent = Q_NULLPTR);
    /// Destructor
    virtual ~Client();

    /// Set client socket
    void setSocket(QTcpSocket *socket);

    /// Set client ID
    void setID(qintptr id);

protected:

    /// Clinet name
    QString     name;
    /// Client ID
    qintptr     id;
    /// Client socket
    QTcpSocket  *socket;
};

#endif // CLIENT_H
