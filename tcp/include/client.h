//------------------------------------------------------------------------------
//
//
//
//
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief
 * \copyright
 * \author
 * \date
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

protected:

    /// Clinet name
    QString     name;
    /// Client ID
    qintptr     id;
    /// Client socket
    QTcpSocket  *socket;
};

#endif // CLIENT_H
