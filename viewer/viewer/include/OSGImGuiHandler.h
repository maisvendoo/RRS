#ifndef     OSG_IMGUI_HANDLER_H
#define     OSG_IMGUI_HANDLER_H

#include    <osgViewer/ViewerEventHandlers>

namespace osg
{
    class Camera;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class OSGImGuiHandler : public osgGA::GUIEventHandler
{
public:

    OSGImGuiHandler();

    bool handle(const osgGA::GUIEventAdapter &a,
                osgGA::GUIActionAdapter &aa) override;

protected:

    virtual void drawUI() = 0;

private:

    void init();

    void setCameraCallbacks(osg::Camera *camera);

    void newFrame(osg::RenderInfo &renderInfo);

    void render(osg::RenderInfo &renderInfo);

private:

    struct ImGuiNewFrameCallback;

    struct ImGuiRenderCallback;

    double time = 0;

    bool mousePressed[3] = {false};

    float mouseWheel = 0;

    bool initialized = false;
};

#endif
