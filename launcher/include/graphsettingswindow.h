#ifndef     GRAPHSETTINGSWINDOW_H
#define     GRAPHSETTINGSWINDOW_H

#include    <QMainWindow>
#include    <gpu-info.h>

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

    Ui::GraphSettingsWindow *ui;
};

#endif // GRAPHSETTINGSWINDOW_H
