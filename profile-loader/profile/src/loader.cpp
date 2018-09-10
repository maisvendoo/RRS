#include    "loader.h"

#include    <QFile>
#include    <QLibrary>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Loader::Loader(QObject *parent) : QObject(parent)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Loader::~Loader()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
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

    profile_t profile_data;

    foreach (QString line, file_content)
    {
        profile_element_t *profile_element = getProfileElement(line);

        if (profile_element != Q_NULLPTR)
            profile_data.push_back(profile_element);
    }

    return profile_data;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Loader *loadLoader(QString lib_path)
{
    Loader *loader = nullptr;

    QLibrary lib(lib_path);

    if (lib.load())
    {
        GetLoader getLoader = (GetLoader) lib.resolve("getLoader");

        if (getLoader)
        {
            loader = getLoader();
        }
    }

    return loader;
}
