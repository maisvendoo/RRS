#ifndef ROUTE_EDITOR_H
#define ROUTE_EDITOR_H

#include "editor/EditorContext.h"

class WindowHandler;

class RouteEditor
{
public:
    RouteEditor();

    ~RouteEditor();

    bool initialize();

    void run();

private:
    void read_settings(const char* filename);

    void configure_shaders();

    void compile_models();

    void handle_deferred_selection();

private:
    EditorContext editor_ctx;
    vsg::ref_ptr<WindowHandler> window_handler_;
};

#endif // ROUTE_EDITOR_H
