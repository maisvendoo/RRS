#ifndef     IMGUI_WIDGETS_HANDLER_H
#define     IMGUI_WIDGETS_HANDLER_H

#include    <OSGImGuiHandler.h>

class ImGuiWidgetsHandler : public OSGImGuiHandler
{
protected:

    void drawUI() override;
private:

    bool is_quit = false;

    void showQuitDialog();
};

#endif
