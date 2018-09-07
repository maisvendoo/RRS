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

#include    "log.h"

#include    <QDate>
#include    <QTime>

#define     HEADER_SYMBOLS      80

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Log::Log(const QString &path, bool clean, bool print_header)
{
    // File creation
    logFile = new QFile(path);

    // Try open file
    bool isOpened = false;

    if (clean)
        isOpened = logFile->open(QIODevice::WriteOnly | QIODevice::Text);
    else
        isOpened = logFile->open(QIODevice::Append | QIODevice::Text);

    if (!isOpened)
        return;

    // Write log header

    // Current date and time
    QDate date = QDate::currentDate();
    QTime time = QTime::currentTime();

    QTextStream out(logFile);

    if (print_header)
    {
        for (int i = 0; i < HEADER_SYMBOLS; i++)
            out << "-";

        out << "\n\n";

        QString text = "\tSession at " +
                date.toString() + " " +
                time.toString() + "\n\n";

        out << text;

        for (int i = 0; i < HEADER_SYMBOLS; i++)
            out << "-";

        out << "\n";
    }

    // Try close file
    logFile->close();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Log::~Log()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Log::printMessage(QString msg)
{
    if (!logFile->open(QIODevice::Append | QIODevice::Text))
        return;

    QTextStream out(logFile);

    out << msg << "\n";

    logFile->close();
}
