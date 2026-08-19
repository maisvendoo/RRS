#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <QDir>

#include <string>

#ifdef FILESYSTEM_LIB
    #define FILESYSTEM_EXPORT Q_DECL_EXPORT
#else
    #define FILESYSTEM_EXPORT Q_DECL_IMPORT
#endif

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class FILESYSTEM_EXPORT FileSystem
{
public:
    /// Get instance by filesystem singleton
    static FileSystem& getInstance();

    /// Get directory by num_levels levels up
    std::string getLevelUpDirectory(const std::string& path,
        int num_levels) const;

    std::string getNativePath(const std::string& path) const;

    /// Get route directory path
    const std::string& getRouteRootDir() const;

    const std::string& getConfigDir() const;

    const std::string& getDocsDir() const;

    const std::string& getLogsDir() const;

    const std::string& getLibraryDir() const;

    const std::string& getTrainsDir() const;

    const std::string& getModulesDir() const;

    const std::string& getVehiclesDir() const;

    const std::string& getCouplingsDir() const;

    const std::string& getDevicesDir() const;

    const std::string& getBinaryDir() const;

    const std::string& getPluginsDir() const;

    const std::string& getDataDir() const;

    const std::string& getVehicleModelsDir() const;

    const std::string& getVehicleTexturesDir() const;

    const std::string& getScreenshotsDir() const;

    const std::string& getFontsDir() const;

    const std::string& getSoundsDir() const;

    const std::string& getThemeDir() const;

    std::string combinePath(const std::string& path1,
        const std::string& path2) const;

    template <typename... Args>
    std::string combinePath(const std::string& path1,
        const std::string& path2, Args&... args) const
    {
        return combinePath(combinePath(path1, path2), args...);
    }

    std::string toNativeSeparators(const std::string& path) const;

    /// Get native path separator
    char separator() const;

private:
    std::string routeRootDir;
    std::string configDir;
    std::string docsDir;
    std::string logsDir;
    std::string libraryDir;
    std::string trainsDir;
    std::string modulesDir;
    std::string vehiclesDir;
    std::string couplingsDir;
    std::string devicesDir;
    std::string binDir;
    std::string pluginsDir;

    std::string dataDir;
    std::string vehicleModelsDir;
    std::string vehicleTexturesDir;

    std::string screenshotsDir;
    std::string fontsDir;

    std::string soundsDir;

    std::string themeDir;

    FileSystem();
    FileSystem(const FileSystem&) = delete;
    FileSystem& operator=(FileSystem&) = delete;

    /// Set directory path in platform native format
    void setDir(std::string& dir, const std::string& path);
};

#endif
