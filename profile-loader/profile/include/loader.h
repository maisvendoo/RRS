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

    virtual profile_element_t *getProfileElement() = 0;
};

#endif // LOADER_H
