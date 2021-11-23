#include    "filesystem.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void FileSystem::setRouteRootDir(const std::string &path)
{
    routeRootDir = getNativePath(path);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void FileSystem::setConfigDir(const std::string &path)
{
    configDir = getNativePath(path);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void FileSystem::setLogsDir(const std::string &path)
{
    logsDir = getNativePath(path);
}

void FileSystem::setLibraryDir(const std::string &path)
{
    libraryDir = getNativePath(path);
}

void FileSystem::setTrainsDir(const std::string &path)
{
    trainsDir = getNativePath(path);
}

void FileSystem::setModulesDir(const std::string &path)
{
    modulesDir = getNativePath(path);
}

void FileSystem::setVehiclesDir(const std::string &path)
{
    vehiclesDir = getNativePath(path);
}

void FileSystem::setCouplingsDir(const std::string &path)
{
    couplingsDir = getNativePath(path);
}

void FileSystem::setDevicesDir(const std::string &path)
{
    devicesDir = getNativePath(path);
}

void FileSystem::setBinaryDir(const std::string &path)
{
    binDir = getNativePath(path);
}

void FileSystem::setPluginsDir(const std::string &path)
{
    pluginsDir = getNativePath(path);
}

void FileSystem::setDataDir(const std::string &path)
{
    dataDir = getNativePath(path);
}

void FileSystem::setVehicleModelsDir(const std::string &path)
{
    vehicleModelsDir = getNativePath(path);
}

void FileSystem::setVehicleTexturesDir(const std::string &path)
{
    vehicleTexturesDir = getNativePath(path);
}

void FileSystem::setScreenshotsDir(const std::string &path)
{
    screenshotsDir = getNativePath(path);
}

void FileSystem::setFontsDir(const std::string &path)
{
    fontsDir = getNativePath(path);
}

void FileSystem::setSoundsDir(const std::string &path)
{
    soundsDir = getNativePath(path);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void FileSystem::setThemeDir(const std::string &path)
{
    themeDir = getNativePath(path);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void FileSystem::setShadersDir(const std::string &path)
{
    shadersDir = getNativePath(path);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::string FileSystem::getRouteRootDir() const
{
    return routeRootDir;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::string FileSystem::getConfigDir() const
{
    return configDir;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::string FileSystem::getLogsDir() const
{
    return logsDir;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::string FileSystem::getLibraryDir() const
{
    return libraryDir;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::string FileSystem::getTrainsDir() const
{
    return trainsDir;
}

std::string FileSystem::getModulesDir() const
{
    return modulesDir;
}

std::string FileSystem::getVehiclesDir() const
{
    return vehiclesDir;
}

std::string FileSystem::getCouplingsDir() const
{
    return couplingsDir;
}

std::string FileSystem::getDevicesDir() const
{
    return devicesDir;
}

std::string FileSystem::getBinaryDir() const
{
    return binDir;
}

std::string FileSystem::getPluginsDir() const
{
    return  pluginsDir;
}

std::string FileSystem::getDataDir() const
{
    return dataDir;
}

std::string FileSystem::getVehicleModelsDir() const
{
    return vehicleModelsDir;
}

std::string FileSystem::getVehicleTexturesDir() const
{
    return vehicleTexturesDir;
}

std::string FileSystem::getScreenshotsDir() const
{
    return screenshotsDir;
}

std::string FileSystem::getFontsDir() const
{
    return fontsDir;
}

std::string FileSystem::getSoundsDir() const
{
    return soundsDir;
}

std::string FileSystem::getThemeDir() const
{
    return themeDir;
}

std::string FileSystem::getShadersDir() const
{
    return shadersDir;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::string FileSystem::combinePath(const std::string &path1, const std::string &path2)
{
    if (*(path1.end() - 1) != separator())
        return getNativePath(path1 + separator() + path2);
    else
        return getNativePath(path1 + path2);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::string FileSystem::toNativeSeparators(const std::string &path)
{
    std::string tmp = path;

#if __unix__
    std::replace(tmp.begin(), tmp.end(), '\\', '/');
#else
    std::replace(tmp.begin(), tmp.end(), '/', '\\');
#endif

    return tmp;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::string FileSystem::getNativePath(const std::string &path)
{
    return QDir::toNativeSeparators(QString(path.c_str())).toStdString();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
char FileSystem::separator() const
{
    return QDir::separator().toLatin1();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::string FileSystem::getLevelUpDirectory(std::string path, int num_levels)
{
    QDir dir(QString(path.c_str()));

    for (int i = 0; i < num_levels; ++i)
        dir.cdUp();

    QString tmp = dir.path() + QDir::separator();

    return tmp.toStdString();
}
