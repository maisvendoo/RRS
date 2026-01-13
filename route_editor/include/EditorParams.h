#ifndef EDITOR_PARAMS_H
#define EDITOR_PARAMS_H

#include "EditorState.h"
#include "RouteMap.h"
#include "StringMap.h"

#include <vsg/core/Inherit.h>
#include <vsg/core/Object.h>
#include <vsg/core/ref_ptr.h>

#include <filesystem>
#include <vsg/ui/KeyEvent.h>

class Topology;

namespace vsg
{

class MatrixTransform;
class Perspective;

}

struct EditorParams : public vsg::Inherit<vsg::Object, EditorParams>
{
    bool show_demo_window = false;
    EditorState* editor_state = nullptr;
    const vsg::KeySymbol* key_bindings = nullptr;
    std::filesystem::path route_dir;
    const StringMap* objects_ref = nullptr;
    const RouteMap* route_map = nullptr;
    vsg::ref_ptr<vsg::MatrixTransform>* selected_object = nullptr;
    vsg::ref_ptr<vsg::Perspective> perspective = nullptr;
    Topology* topology = nullptr;
};

#endif // EDITOR_PARAMS_H
