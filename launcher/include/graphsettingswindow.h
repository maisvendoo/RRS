#ifndef     GRAPHSETTINGSWINDOW_H
#define     GRAPHSETTINGSWINDOW_H

#include    <QMainWindow>
#include    <gpu-info.h>
#include    <CfgEditor.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
namespace Ui
{
    class GraphSettingsWindow;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class GraphSettingsWindow : public QMainWindow
{
    Q_OBJECT

public:

    explicit GraphSettingsWindow(QWidget *parent = nullptr);

    ~GraphSettingsWindow();

    void setSettingsGPU(const gpus_info_list_t &gpus_info);

private:

    gpus_info_list_t gpus_info;

    int current_gpu_idx = -1;

    Ui::GraphSettingsWindow *ui;

    FieldsDataList fd_list;

    QString settings_path = "";

    static const   QString WIDTH;
    static const   QString HEIGHT;
    static const   QString FULLSCREEN;
    static const   QString FOV_Y;
    static const   QString ZNEAR;
    static const   QString ZFAR;
    static const   QString SCREEN_NUM;
    static const   QString WIN_DECOR;
    static const   QString DOUBLE_BUFF;
    static const   QString VSYNC;
    static const   QString NOTIFY_LEVEL;
    static const   QString VIEW_DIST;
    static const   QString MAX_FPS;
    static const   QString PHYSICAL_DEVICE;
    static const   QString SAMPLES;

    void loadGraphicsSettings(QString file_name);

    void updateGraphSettings(FieldsDataList &fd_list,
                             Ui::GraphSettingsWindow *ui);

    void applyGraphSettings(FieldsDataList &fd_list,
                            Ui::GraphSettingsWindow *ui);

    void saveGraphSettings(FieldsDataList &fd_list);

    void showEvent(QShowEvent *event) override;

private slots:

    void slotOnChangeCurrentGPU(int idx);

    void slotApplySettings();

    void slotCancelSettings();
};

#endif // GRAPHSETTINGSWINDOW_H
