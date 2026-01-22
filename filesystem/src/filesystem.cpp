#include "filesystem.h"

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
    const std::string workDir = QDir::currentPath().toStdString();
    const std::string tmp = getLevelUpDirectory(workDir, 1);

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
const std::string& FileSystem::getRouteRootDir() const
{
    return routeRootDir;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
const std::string& FileSystem::getConfigDir() const
{
    return configDir;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
const std::string& FileSystem::getLogsDir() const
{
    return logsDir;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
const std::string& FileSystem::getLibraryDir() const
{
    return libraryDir;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
const std::string& FileSystem::getTrainsDir() const
{
    return trainsDir;
}

const std::string& FileSystem::getModulesDir() const
{
    return modulesDir;
}

const std::string& FileSystem::getVehiclesDir() const
{
    return vehiclesDir;
}

const std::string& FileSystem::getCouplingsDir() const
{
    return couplingsDir;
}

const std::string& FileSystem::getDevicesDir() const
{
    return devicesDir;
}

const std::string& FileSystem::getBinaryDir() const
{
    return binDir;
}

const std::string& FileSystem::getPluginsDir() const
{
    return  pluginsDir;
}

const std::string& FileSystem::getDataDir() const
{
    return dataDir;
}

const std::string& FileSystem::getVehicleModelsDir() const
{
    return vehicleModelsDir;
}

const std::string& FileSystem::getVehicleTexturesDir() const
{
    return vehicleTexturesDir;
}

const std::string& FileSystem::getScreenshotsDir() const
{
    return screenshotsDir;
}

const std::string& FileSystem::getFontsDir() const
{
    return fontsDir;
}

const std::string& FileSystem::getSoundsDir() const
{
    return soundsDir;
}

const std::string& FileSystem::getThemeDir() const
{
    return themeDir;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::string FileSystem::combinePath(const std::string& path1,
    const std::string& path2) const
{
    if (path1.empty())
    {
        return path2;
    }

    if (path1.back() != separator())
    {
        return getNativePath(path1 + separator() + path2);
    }
    else
    {
        return getNativePath(path1 + path2);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::string FileSystem::toNativeSeparators(const std::string& path) const
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
std::string FileSystem::getNativePath(const std::string& path) const
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
std::string FileSystem::getLevelUpDirectory(const std::string& path,
    int num_levels) const
{
    QDir dir(QString(path.c_str()));

    for (int i = 0; i < num_levels; ++i)
    {
        dir.cdUp();
    }

    QString tmp = dir.path() + QDir::separator();

    return tmp.toStdString();
}
