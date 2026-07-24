#ifndef UNDO_REDO_SAVE_HANDLER_H
#define UNDO_REDO_SAVE_HANDLER_H

#include "RouteObject.h"

#include <vsg/core/Inherit.h>
#include <vsg/core/Visitor.h>
#include <vsg/core/ref_ptr.h>

#include <mutex>
#include <string>

class CommandList;
class Keyboard;

namespace vsg
{

class KeyPressEvent;

}

// This is temp class to move out functionality from Keyboard
class UndoRedoSaveHandler : public vsg::Inherit<vsg::Visitor, UndoRedoSaveHandler>
{
public:
    UndoRedoSaveHandler(
        const vsg::ref_ptr<Keyboard>& keyboard,
        CommandList& commands,
        const std::string& route_dir,
        std::mutex& static_objects_mutex,
        const RouteObjects& static_objects
    );

    virtual void apply(vsg::KeyPressEvent& keyPress) override;

private:
    void save_route() const;

private:
    const vsg::ref_ptr<Keyboard>& keyboard_;
    CommandList& commands_;
    const std::string& route_dir_;
    std::mutex& static_objects_mutex_;
    const RouteObjects& static_objects_;
};

#endif // UNDO_REDO_SAVE_HANDLER_H
