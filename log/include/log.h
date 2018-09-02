//------------------------------------------------------------------------------
//
//      Print messages to log-file
//      (с) maisvendoo, 23/06/2017
//      Developer: Dmitry Pritykin
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief Print messages to log-file
 * \copyright maisvendoo
 * \author Dmitry Pritykin
 * \date 23/06/2017
 */

#ifndef LOG_H
#define LOG_H

#include    <QObject>
#include    <QFile>
#include    <QTextStream>

#if defined(LOG_LIB)
    #define LOG_EXPORT Q_DECL_EXPORT
#else
    #define LOG_EXPORT Q_DECL_IMPORT
#endif

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class LOG_EXPORT Log : public QObject
{
    Q_OBJECT

public:

    /*!
     * \brief
     * \param path - log-file path
     * \param clean - celar log-file
     */
    Log(const QString &path, bool clean = false, bool print_header = true);

    /// Деструктор
    virtual ~Log();

public slots:

    /*!
     * \brief
     * \param msg - text of the message
     */
    void printMessage(QString msg);

private:

    /// Log-file object
    QFile   *logFile;
};

#endif // LOG_H
