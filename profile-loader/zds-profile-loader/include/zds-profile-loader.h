//------------------------------------------------------------------------------
//
//
//
//
//
//------------------------------------------------------------------------------
#ifndef     ZDS_PROFILE_LOADER_H
#define     ZDS_PROFILE_LOADER_H

#include    "loader.h"
#include    "zds-profile.h"

#include    <QStringList>

class ZdsProfileLoader : public Loader
{
public:

    ZdsProfileLoader(QObject *parent = Q_NULLPTR);
    ~ZdsProfileLoader();

private:

    profile_element_t *getProfileElement(QString line);

    zds_profile_element_t parse(QStringList tokens);
};

#endif // ZDS_PROFILE_LOADER_H
