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
    static FileSystem &getInstance()
    {
        static FileSystem instance;

        std::string workDir = QDir::currentPath().toStdString();
        std::string tmp = instance.getLevelUpDirectory(workDir, 1);
        instance.setBinaryDir(workDir);
        instance.setRouteRootDir(tmp + "routes");
        instance.setConfigDir(tmp + "cfg");
        instance.setLogsDir(tmp + "logs");
        instance.setLibraryDir(tmp + "lib");
        instance.setTrainsDir(instance.getConfigDir() + instance.separator() + "trains");
        instance.setModulesDir(tmp + "modules");
        instance.setVehiclesDir(instance.getConfigDir() + instance.separator() + "vehicles");
        instance.setCouplingsDir(instance.getConfigDir()+ instance.separator() + "couplings");
        instance.setDevicesDir(instance.getConfigDir()+ instance.separator() + "devices");
        instance.setDataDir(tmp + "data");
        instance.setVehicleModelsDir(instance.combinePath(instance.getDataDir(), "models"));
        instance.setVehicleTexturesDir(instance.combinePath(instance.getDataDir(), "textures"));
        instance.setPluginsDir(tmp + "plugins");
        instance.setScreenshotsDir(tmp + "screenshots");
        instance.setFontsDir(tmp + "fonts");
        instance.setSoundsDir(instance.combinePath(instance.getDataDir(), "sounds"));

        return instance;
    }    

    std::string getNativePath(const std::string &path);

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

    std::string combinePath(const std::string &path1, const std::string &path2);

    std::string toNativeSeparators(const std::string &path);

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

    FileSystem() {}
    FileSystem(const FileSystem &) = delete;
    FileSystem &operator=(FileSystem &) = delete;

    /// Set route direcory path in paltform native format
    void setRouteRootDir(const std::string &path);

    /// Set config directory path
    void setConfigDir(const std::string &path);

    void setLogsDir(const std::string &path);

    void setLibraryDir(const std::string &path);

    void setTrainsDir(const std::string &path);

    void setModulesDir(const std::string &path);

    void setVehiclesDir(const std::string &path);

    void setCouplingsDir(const std::string &path);

    void setDevicesDir(const std::string &path);

    void setBinaryDir(const std::string &path);

    void setPluginsDir(const std::string &path);

    void setDataDir(const std::string &path);

    void setVehicleModelsDir(const std::string &path);

    void setVehicleTexturesDir(const std::string &path);

    void setScreenshotsDir(const std::string &path);

    void setFontsDir(const std::string &path);

    void setSoundsDir(const std::string &path);

    /// Get directory by num_levels levels up
    std::string getLevelUpDirectory(std::string path, int num_levels);
};

#endif
