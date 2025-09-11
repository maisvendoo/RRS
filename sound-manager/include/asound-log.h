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

#include <QObject>

#include <fstream>

class LogFileHandler : public QObject
{
public:
    LogFileHandler(const std::string& dir, const std::string& file);

    virtual ~LogFileHandler();

public slots:
    /// Log message handler
    virtual void notify(const std::string& msg);

protected:
    /// Log output stream
    std::ofstream log_;
};

#endif // ASOUNDLOG_H
