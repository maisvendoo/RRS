#include    "data-prepare.h"

DataPrepare::DataPrepare() : AbstractDataEngine()
{

}

QByteArray DataPrepare::getPreparedData()
{
    QMutexLocker locker(&outMutex_);
    return outputBuffer_;
}
