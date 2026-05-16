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
    static const   QString DEPTH_FORMAT;
    static const   QString SHADOW;
    static const   QString SHADOWS_PRESET;
    static const   QString SHADOW_DISTANCE;
    static const   QString SHADOW_CASCADE;
    static const   QString SHADOW_RESOLUTION;

    void loadGraphicsSettings(QString file_name);

    void updateGraphSettings(FieldsDataList &fd_list,
                             Ui::GraphSettingsWindow *ui);

    void applyGraphSettings(FieldsDataList &fd_list,
                            Ui::GraphSettingsWindow *ui);

    void saveGraphSettings(FieldsDataList &fd_list);

    void showEvent(QShowEvent *event) override;

    void setDefaultSettingsForChangedGPU(int gpu_idx);

    struct ShadowsPreset
    {
        size_t  resolution_idx = 0;
        int     cascades = 1;
        int     distanse = 20;
    };

    std::vector<uint32_t> shadowMapResolution = {1024, 2048, 4096};

    std::vector<ShadowsPreset> shadowsPresets = {
        {0, 1, 30},
        {1, 2, 80},
        {2, 3, 150},
        {2, 4, 300}
    };

    void ulockShadowsSettings(Ui::GraphSettingsWindow *ui, bool lock);

private slots:

    void slotOnChangeCurrentGPU(int idx);

    void slotApplySettings();

    void slotCancelSettings();
};

#endif // GRAPHSETTINGSWINDOW_H
