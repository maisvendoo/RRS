#ifndef ROUTE_VIEWER_H
#define ROUTE_VIEWER_H

#include "SoundManager.h"
#include "TrainExteriorHandler.h"
#include "settings.h"

#include <memory>
#include <string>
#include <vsg/app/RecordTraversal.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/nodes/Group.h>

class RouteViewer
{
public:
    RouteViewer(int argc, char* argv[]);

    bool isReady() const;

    int run();

private:
    bool init(int argc, char* argv[]);

    void loadSettings(const std::string& cfg_path);

    int overrideSettingsByCommandLine(int argc, char* argv[]);

    bool initEngineSettings();

private:
    bool is_ready;

    settings_t settings;

    std::unique_ptr<SoundManager> sound_manager;

    std::unique_ptr<TrainExteriorHandler> train_ext_handler;

    vsg::ref_ptr<vsg::Group> root;
};

#endif // ROUTE_VIEWER_H
