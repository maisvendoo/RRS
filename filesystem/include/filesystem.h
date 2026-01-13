#ifndef		FILESYSTEM_H
#define		FILESYSTEM_H

#include    <QDir>

#ifdef FILESYSTEM_LIB
    #define FILESYSTEM_EXPORT   Q_DECL_EXPORT
#else
    #define FILESYSTEM_EXPORT   Q_DECL_IMPORT
#endif

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class FILESYSTEM_EXPORT FileSystem
{
public:

    /// Get instance byt filesystem singleton
    static FileSystem &getInstance();

    /// Get directory by num_levels levels up
    std::string getLevelUpDirectory(std::string path, int num_levels) const;

    std::string getNativePath(const std::string &path) const;

    /// Get route directory path
    std::string getRouteRootDir() const;

    std::string getConfigDir() const;

    std::string getLogsDir() const;

    std::string getLibraryDir() const;

    std::string getTrainsDir() const;

    std::string getModulesDir() const;

    std::string getVehiclesDir() const;

    std::string getCouplingsDir() const;

    std::string getDevicesDir() const;

    std::string getBinaryDir() const;

    std::string getPluginsDir() const;

    std::string getDataDir() const;

    std::string getVehicleModelsDir() const;

    std::string getVehicleTexturesDir() const;

    std::string getScreenshotsDir() const;

    std::string getFontsDir() const;

    std::string getSoundsDir() const;

    std::string getThemeDir() const;

    std::string combinePath(const std::string &path1, const std::string &path2) const;

    std::string toNativeSeparators(const std::string &path) const;

    /// Get native path separator
    char separator() const;

private:

    std::string routeRootDir;
    std::string configDir;
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
    FileSystem(const FileSystem &) = delete;
    FileSystem &operator=(FileSystem &) = delete;

    /// Set directory path in platform native format
    void setDir(std::string& dir, const std::string& path);
};

#endif
