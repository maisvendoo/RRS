#include    "filesystem.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
FileSystem& FileSystem::getInstance()
{
    static FileSystem instance;
    return instance;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
FileSystem::FileSystem()
{
    std::string workDir = QDir::currentPath().toStdString();
    std::string tmp = getLevelUpDirectory(workDir, 1);

    setDir(binDir, workDir);
    setDir(routeRootDir, tmp + "routes");
    setDir(configDir, tmp + "cfg");
    setDir(logsDir, tmp + "logs");
    setDir(libraryDir, tmp + "lib");
    setDir(trainsDir, configDir + separator() + "trains");
    setDir(modulesDir, tmp + "modules");
    setDir(vehiclesDir, configDir + separator() + "vehicles");
    setDir(couplingsDir, configDir + separator() + "couplings");
    setDir(devicesDir, configDir + separator() + "devices");
    setDir(dataDir, tmp + "data");
    setDir(vehicleModelsDir, combinePath(dataDir, "models"));
    setDir(vehicleTexturesDir, combinePath(dataDir, "textures"));
    setDir(pluginsDir, tmp + "plugins");
    setDir(screenshotsDir, tmp + "screenshots");
    setDir(fontsDir, tmp + "fonts");
    setDir(soundsDir, combinePath(dataDir, "sounds"));
    setDir(themeDir, tmp + "themes");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void FileSystem::setDir(std::string& dir, const std::string& path)
{
    dir = getNativePath(path);
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
