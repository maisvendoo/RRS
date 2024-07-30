#ifndef     IMGUI_WIDGETS_HANDLER_H
#define     IMGUI_WIDGETS_HANDLER_H

#include    <OSGImGuiHandler.h>

class ImGuiWidgetsHandler : public OSGImGuiHandler
{

public:

    bool handle(const osgGA::GUIEventAdapter &ea,
                osgGA::GUIActionAdapter &aa) override;

protected:

    void drawUI() override;
private:

    bool is_show_quit_dialog = false;

    void showQuitDialog(bool &is_show);
};

#endif
