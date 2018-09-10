//------------------------------------------------------------------------------
//
//
//
//
//
//------------------------------------------------------------------------------
#ifndef     PROFILE_H
#define     PROFILE_H

#include    <QObject>

#include    "filesystem.h"

#include    "profile-export.h"
#include    "profile-element.h"
#include    "loader.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class PROFILE_EXPORT Profile : public QObject
{
    Q_OBJECT

public:

    explicit Profile(FileSystem *fs, QObject *parent = Q_NULLPTR);
    virtual ~Profile();

    bool init(QString profile_path);

private:

    FileSystem  *fs;

    /// Profile's data
    profile_t   profile_data;

    /// Profile loader
    Loader      *loader;

    Loader  *selectLoader(QString profile_path);
};

#endif // PROFILE_H
