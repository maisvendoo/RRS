#include    "path-funcs.h"

#include    <QDir>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString toNativePath(const QString &path)
{
    QString tmp = path;

#if defined(Q_OS_UNIX)
    tmp.replace('\\', QDir::separator());
#else
    tmp.replace('/', QDir::separator());
#endif

    if (*(tmp.begin()) == QDir::separator())
        tmp.remove(0, 1);

    return tmp;
}
