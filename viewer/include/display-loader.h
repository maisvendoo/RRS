// #ifndef     DISPLAY_LOADER_H
// #define     DISPLAY_LOADER_H

// #include    "display-container.h"

// #include    <osg/Node>

// #include    "display-config.h"

// void loadDisplayModule(const display_config_t &display_config,
//                         display_container_t *dc,
//                         osg::Node *model);

// #endif // DISPLAY_LOADER_H

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#ifndef DISPLAY_LOADER_H
#define DISPLAY_LOADER_H

#include "display-config.h"
#include "display-container.h"

#include <vsg/nodes/MatrixTransform.h>

void loadDisplayModule(
    const display_config_t& display_config,
    display_container_t* dc,
    vsg::ref_ptr<vsg::Node> model
);

#endif // DISPLAY_LOADER_H
