#ifndef ROUTE_EDITOR_H
#define ROUTE_EDITOR_H

#include "EditorContext.h"

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
};

#endif // ROUTE_EDITOR_H
