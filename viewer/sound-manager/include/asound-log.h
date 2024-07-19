//------------------------------------------------------------------------------
//
//      ASound log messages handler
//      (c) DimaGVRH, 09/01/2020
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief ASound log messages handler
 * \copyright DimaGVRH
 * \author DimaGVRH
 * \date 09/01/2020
 */

#ifndef ASOUNDLOG_H
#define ASOUNDLOG_H

#include    <fstream>
#include    <QTimer>
#include    <QFile>

class LogFileHandler : public QObject
{   
public:

    /// Constructor
    LogFileHandler(const std::string &file);

    /// Destructor
    virtual ~LogFileHandler();

public slots:
    /// Log message handler
    virtual void notify(const std::string msg);

protected:

    /// Log output stream
    std::ofstream log_;


private:

    /// File opened flag
    bool canDo_;

    /// Log file
    QFile* file_;
};

#endif // ASOUNDLOG_H
