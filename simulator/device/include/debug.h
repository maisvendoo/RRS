//------------------------------------------------------------------------------
//
//
//
//
//------------------------------------------------------------------------------
#ifndef     DEBUG_H
#define     DEBUG_H

#include    <QObject>
#include    <fstream>

#include    "solver-types.h"

#if defined(DEVICE_LIB)
    #define DEVICE_EXPORT   Q_DECL_EXPORT
#else
    #define DEVICE_EXPORT   Q_DECL_IMPORT
#endif

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DEVICE_EXPORT DebugLog : public QObject
{
    Q_OBJECT

public:

    DebugLog(const std::string &path, QObject *parent = Q_NULLPTR);

    ~DebugLog();

public slots:

    void DebugPring(double t, const state_vector_t &Y);

private:

    std::ofstream   log;
};

#endif // DEBUG_H
