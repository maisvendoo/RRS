#ifndef SAVE_HANDLER_H
#define SAVE_HANDLER_H

#include "RouteObjects.h"

#include <vsg/core/Inherit.h>
#include <vsg/core/Visitor.h>
#include <vsg/core/ref_ptr.h>

#include <mutex>
#include <string>

class Keyboard;

namespace vsg
{

class KeyPressEvent;

}

// This is temp class to move out functionality from Keyboard
class SaveHandler : public vsg::Inherit<vsg::Visitor, SaveHandler>
{
public:
    SaveHandler(
        const vsg::ref_ptr<Keyboard>& keyboard,
        const std::string& route_dir,
        std::mutex& static_objects_mutex,
        const RouteObjects& static_objects
    );

    virtual void apply(vsg::KeyPressEvent& keyPress) override;

private:
    void save_route() const;

private:
    const vsg::ref_ptr<Keyboard>& keyboard_;
    const std::string& route_dir_;
    std::mutex& static_objects_mutex_;
    const RouteObjects& static_objects_;
};

#endif // SAVE_HANDLER_H
