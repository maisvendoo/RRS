#ifndef     OSG_IMGUI_HANDLER_H
#define     OSG_IMGUI_HANDLER_H

#include    <osgViewer/ViewerEventHandlers>
#include    <osg/Camera>

#include    <imgui.h>
#include    <imgui_impl_opengl3.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class OSGImGuiHandler : public osgGA::GUIEventHandler
{
public:

    OSGImGuiHandler();

    bool handle(const osgGA::GUIEventAdapter &ea,
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

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class ImGuiInitOperation : public osg::Operation
{
public:

    ImGuiInitOperation() : osg::Operation("ImGuiInitOperation", false)
    {

    }

    void operator()(osg::Object *object) override
    {
        osg::GraphicsContext *context = dynamic_cast<osg::GraphicsContext *>(object);

        if (!context)
            return;

        if (!ImGui_ImplOpenGL3_Init())
        {
            OSG_FATAL << "FAILED: ImGui_ImplOpenGL3_Init() failed\n";
        }
    }
};

#endif
