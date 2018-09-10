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

#include    "profile-export.h"
#include    "profile-element.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class PROFILE_EXPORT Profile : public QObject
{
    Q_OBJECT

public:

    explicit Profile(QObject *parent = Q_NULLPTR);
    virtual ~Profile();

private:

    /// Profile's data
    profile_t   profile_data;

};

#endif // PROFILE_H
