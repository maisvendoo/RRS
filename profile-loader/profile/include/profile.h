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

    explicit Profile(FileSystem *fs, double prof_step = 100.0, QObject *parent = Q_NULLPTR);
    virtual ~Profile();

    bool init(QString profile_path);

private:

    /// Pointer to filesystem object
    FileSystem  *fs;

    /// Profile step
    double prof_step;

    /// Zero element of profile
    const profile_element_t ZERO_ELEMENT;

    /// Profile's data
    profile_t   profile_data;

    /// Profile loader
    Loader      *loader;

    /// Select loader by profile type
    Loader  *selectLoader(QString profile_path);
};

#endif // PROFILE_H
