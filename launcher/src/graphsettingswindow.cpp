#include    "graphsettingswindow.h"
#include    "ui_graphsettingswindow.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
GraphSettingsWindow::GraphSettingsWindow(QWidget *parent) : QMainWindow(parent)
    , ui(new Ui::GraphSettingsWindow)
{
    ui->setupUi(this);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
GraphSettingsWindow::~GraphSettingsWindow()
{
    delete ui;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void GraphSettingsWindow::setSettingsGPU(const gpus_info_list_t &gpus_info)
{
    this->gpus_info = gpus_info;

    if (gpus_info.empty())
    {
        return;
    }

    int max_score = 0;
    int best_gpu_idx = -1;

    for (size_t i = 0; i < gpus_info.size(); ++i)
    {
        auto &gpu_info = gpus_info[i];

        if (gpu_info.score > max_score)
        {
            max_score = gpu_info.score;
            best_gpu_idx = i;
        }

        ui->cbListGPU->addItem(gpu_info.deviceName);
        ui->cbListGPU->setCurrentIndex(best_gpu_idx);
    }
}
