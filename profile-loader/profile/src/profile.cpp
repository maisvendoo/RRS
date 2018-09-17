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

    profile_raw_data = loader->load(profile_path);
    profile_data = processData(prof_step);

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

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
profile_t Profile::processData(double prof_step)
{
    profile_t data;

    auto end = profile_raw_data.end() - 1;

    for (auto it = profile_raw_data.begin(); it != end; ++it)
    {
        profile_element_t *prof_element = *it;
        profile_element_t *next_prof_element = *(it+1);

        double x = prof_element->railway_coord;
        double x_end = next_prof_element->railway_coord;

        double dx = prof_step;

        if (x <= x_end)
        {
            while (x < x_end)
            {
                profile_element_t *prof_elem = new profile_element_t();
                prof_elem->railway_coord = x;
                prof_elem->inclination = prof_element->inclination;
                prof_elem->curvature = prof_element->curvature;

                data.push_back(prof_elem);

                x += dx;
            }
        }
        else
        {
            while (x > x_end)
            {
                profile_element_t *prof_elem = new profile_element_t();
                prof_elem->railway_coord = x;
                prof_elem->inclination = prof_element->inclination;
                prof_elem->curvature = prof_element->curvature;

                data.push_back(prof_elem);

                x -= dx;
            }
        }
    }

    return data;
}
