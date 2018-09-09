#include    "loader.h"

#include    <QFile>

Loader::Loader(QObject *parent) : QObject(parent)
{

}

Loader::~Loader()
{

}

profile_t Loader::load(QString profile_path)
{
    QFile file(profile_path);

    if (file.open(QIODevice::ReadOnly))
    {
        while (!file.atEnd())
        {
            file_content.append(file.readLine());
        }
    }
}
