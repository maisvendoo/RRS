#include    "zds-profile-loader.h"

const   QChar   COLUMN_SEPARATOR = '\t';

ZdsProfileLoader::ZdsProfileLoader(QObject *parent) : Loader(parent)
{

}

ZdsProfileLoader::~ZdsProfileLoader()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
profile_element_t *ZdsProfileLoader::getProfileElement(QString line)
{
    if (line.at(0) == ';')
       return Q_NULLPTR;

    profile_element_t *profile_element = new profile_element_t();

    QString tmp = line.replace("\r", "");
    tmp = tmp.replace("\n", "");
    QStringList tokens = tmp.split(COLUMN_SEPARATOR);

    zds_profile_element_t zds_element = parse(tokens);

    profile_element->railway_coord = static_cast<double>(zds_element.ord);
    profile_element->inclination = static_cast<double>(zds_element.uklon);

    if (zds_element.Radius == 0)
        profile_element->curvature = 0.0;
    else
        profile_element->curvature = qAbs(1.0 / static_cast<double>(zds_element.Radius));

    return profile_element;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
zds_profile_element_t ZdsProfileLoader::parse(QStringList tokens)
{
    zds_profile_element_t zds_element;

    zds_element.ord = tokens.at(0).toInt();
    zds_element.uklon = tokens.at(1).toFloat();
    zds_element.Radius = tokens.at(2).toShort();

    return zds_element;
}

GET_LOADER(ZdsProfileLoader)
