#ifndef ROUTE_EDITOR_H
#define ROUTE_EDITOR_H

#include "EditorContext.h"

class Keyboard;
class Mouse;

class RouteEditor
{
public:
    RouteEditor();
    ~RouteEditor();

    bool initialize();
    void run();

private:
    void configure_shaders();
    void compile_models();
    void handle_deferred_selection();

private:
    EditorContext context_;
    vsg::ref_ptr<vsg::Viewer> viewer_;
    vsg::ref_ptr<WindowHandler> window_handler_;
    vsg::ref_ptr<Mouse> mouse;
    vsg::ref_ptr<Keyboard> keyboard;
};

#endif // ROUTE_EDITOR_H
