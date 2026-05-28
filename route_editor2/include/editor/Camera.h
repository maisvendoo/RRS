#ifndef EDITOR_CAMERA_H
#define EDITOR_CAMERA_H

#include <vsg/app/Camera.h>
#include <vsg/core/Inherit.h>
#include <vsg/core/ref_ptr.h>

namespace vsg
{

class LookAt;
class Orthographic;
class Perspective;

}

class Camera : public vsg::Inherit<vsg::Camera, Camera>
{
public:
    Camera();
    ~Camera();

    const vsg::ref_ptr<vsg::Perspective>& get_perspective() const;
    const vsg::ref_ptr<vsg::Orthographic>& get_orthographic() const;
    const vsg::ref_ptr<vsg::LookAt>& get_look_at() const;

private:
    vsg::ref_ptr<vsg::Perspective> perspective;
    vsg::ref_ptr<vsg::Orthographic> orthographic;
    vsg::ref_ptr<vsg::LookAt> look_at;
};

#endif // EDITOR_CAMERA_H
