#ifndef     LOADER_H
#define     LOADER_H

#include    <QObject>
#include    <QString>
#include    <QStringList>

#include    "profile-export.h"
#include    "profile-element.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class PROFILE_EXPORT Loader : public QObject
{
    Q_OBJECT

public:

    explicit Loader(QObject *parent = Q_NULLPTR);
    virtual ~Loader();

    profile_t load(QString profile_path);

protected:

    QStringList     file_content;

    virtual profile_element_t *getProfileElement(QString line) = 0;
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
typedef     Loader* (*GetLoader)();

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
#define GET_LOADER(ClassName) \
    extern "C" Q_DECL_EXPORT Loader *getLoader() \
    {\
        return new (ClassName)(); \
    }

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
extern "C" Q_DECL_EXPORT Loader *loadLoader(QString lib_path);

#endif // LOADER_H
