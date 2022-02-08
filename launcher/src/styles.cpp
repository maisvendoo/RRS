#include    "styles.h"

#include    <QFile>

QString readStyleSheet(QString filename)
{
    QFile file(filename);
    QString css = "";

    if (file.open(QIODevice::ReadOnly))
    {
        css = QString(file.readAll());
        file.close();
    }

    return css;
}
