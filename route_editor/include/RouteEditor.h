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

private:
    EditorContext context_;
};

#endif // ROUTE_EDITOR_H
