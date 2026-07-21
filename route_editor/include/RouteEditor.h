#ifndef ROUTE_EDITOR_H
#define ROUTE_EDITOR_H

#include "EditorContext.h"

#include <memory>

class Keyboard;
class Mouse;
class StateManager;

class RouteEditor
{
public:
    RouteEditor();
    ~RouteEditor();

    bool initialize();
    void run();

private:
    void initialize_journal(const char* filename = "editor.log") const;
    void configure_shaders();
    void compile_models();
    void handle_deferred_selection();

private:
    EditorContext context_;
    vsg::ref_ptr<vsg::Viewer> viewer_;
    vsg::ref_ptr<WindowHandler> window_handler_;
    vsg::ref_ptr<Mouse> mouse;
    vsg::ref_ptr<Keyboard> keyboard;
    std::unique_ptr<StateManager> state_manager;
};

#endif // ROUTE_EDITOR_H
