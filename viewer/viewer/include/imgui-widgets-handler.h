#ifndef     IMGUI_WIDGETS_HANDLER_H
#define     IMGUI_WIDGETS_HANDLER_H

#include    <OSGImGuiHandler.h>
#include    <QObject>
#include    <QString>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class ImGuiWidgetsHandler : public QObject, public OSGImGuiHandler
{    
    Q_OBJECT

public:

    ImGuiWidgetsHandler();

    bool handle(const osgGA::GUIEventAdapter &ea,
                osgGA::GUIActionAdapter &aa) override;

protected:

    void drawUI() override;

private:

    int font_size = 20;

    bool is_show_quit_dialog = false;

    bool is_Esc = false;

    bool is_show_debug_log = false;

    bool is_F1 = false;

    bool is_modified_key = false;

    bool is_controlled = false;

    QString debugMsg = "";

    void showQuitDialog(bool &is_show);

    void showDebugLog();

    void showUncontrolledState();

public slots:

    void setStatusBar(const QString &msg);

    void receiveControlledState(bool state);
};

#endif
