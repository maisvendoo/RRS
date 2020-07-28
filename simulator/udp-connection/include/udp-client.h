//-----------------------------------------------------------------------------
//
//
//
//
//
//-----------------------------------------------------------------------------
/*!
 *  \file
 *  \brief
 *  \copyright
 *  \author
 *  \date
 */

#ifndef UDP_CLIENT_H
#define UDP_CLIENT_H

#include <QtGlobal>
#include <QObject>
#include <QDataStream>

#if defined(UDP_CLIENT_LIB)
# define UDP_CLIENT_EXPORT Q_DECL_EXPORT
#else
# define UDP_CLIENT_EXPORT Q_DECL_IMPORT
#endif

/*!
 *  \class UdpClient
 *  \brief
 */
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
class UDP_CLIENT_EXPORT UdpClient Q_DECL_FINAL : public QObject
{
    Q_OBJECT

public:
    explicit UdpClient();

    ~UdpClient();

    void init();

    bool isConnected();

    void start();

    void stop();

signals:

public slots:

protected:

private:

private slots:

};

#endif // UDP_CLIENT_H
