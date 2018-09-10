#include    "profile.h"

#include    <QFileInfo>

const   QString     ZDS_PROFILE = "dat";

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Profile::Profile(FileSystem *fs, double prof_step, QObject *parent) : QObject(parent)
  , fs(fs)
  , prof_step(prof_step)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Profile::~Profile()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Profile::init(QString profile_path)
{
    loader = selectLoader(profile_path);

    if (loader == Q_NULLPTR)
        return false;

    profile_data = loader->load(profile_path);

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Loader *Profile::selectLoader(QString profile_path)
{
    QFileInfo   info(profile_path);
    QString     ext = info.completeSuffix();
    Loader      *loader = Q_NULLPTR;

    // Get ZDSimulator profile loader
    if (ext == ZDS_PROFILE)
    {
        loader = loadLoader(fs->getLibDirectory() + "zds-profile-loader");
    }

    return loader;
}
