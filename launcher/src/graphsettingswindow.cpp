#include    "graphsettingswindow.h"
#include    "ui_graphsettingswindow.h"

#include    <filesystem.h>
#include    <CfgReader.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
const   QString GraphSettingsWindow::WIDTH = "Width";
const   QString GraphSettingsWindow::HEIGHT = "Height";
const   QString GraphSettingsWindow::FULLSCREEN = "FullScreen";
const   QString GraphSettingsWindow::FOV_Y = "FovY";
const   QString GraphSettingsWindow::ZNEAR = "zNear";
const   QString GraphSettingsWindow::ZFAR = "zFar";
const   QString GraphSettingsWindow::SCREEN_NUM = "ScreenNumber";
const   QString GraphSettingsWindow::WIN_DECOR = "WindowDecoration";
const   QString GraphSettingsWindow::DOUBLE_BUFF = "DoubleBuffer";
const   QString GraphSettingsWindow::VSYNC = "VSync";
const   QString GraphSettingsWindow::NOTIFY_LEVEL = "NofifyLevel";
const   QString GraphSettingsWindow::VIEW_DIST = "ViewDistance";
const   QString GraphSettingsWindow::MAX_FPS = "MaxFPS";
const   QString GraphSettingsWindow::PHYSICAL_DEVICE = "PhysicalDevice";
const   QString GraphSettingsWindow::SAMPLES = "Samples";

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QPair<QString, QVariant> findSetting(QString setting,
                                     FieldsDataList &fd_list,
                                     int &idx)
{
    QPair<QString, QVariant> pair;

    for (int i = 0; i < fd_list.size(); ++i)
    {
        pair = fd_list[i];

        if (pair.first == setting)
        {
            idx = i;
            return pair;
        }
    }

    return pair;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QPair<QString, QVariant> findSetting(QString setting, FieldsDataList &fd_list)
{
    int idx = 0;
    QPair<QString, QVariant> pair = findSetting(setting, fd_list, idx);

    return pair;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
GraphSettingsWindow::GraphSettingsWindow(QWidget *parent) : QMainWindow(parent)
    , ui(new Ui::GraphSettingsWindow)
{
    ui->setupUi(this);

    setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

    connect(ui->pbGraphApply, &QPushButton::released,
            this, &GraphSettingsWindow::slotApplySettings);

    connect(ui->pbGraphCancel, &QPushButton::released,
            this, &GraphSettingsWindow::slotCancelSettings);

    connect(ui->cbWindowDecoration, &QCheckBox::stateChanged, this, [this](int){
        ui->pbGraphApply->setEnabled(true);
    });

    connect(ui->cbFullScreenMode, &QCheckBox::stateChanged, this, [this](int){
        ui->pbGraphApply->setEnabled(true);
    });

    connect(ui->sbWindowWidth, &QSpinBox::valueChanged, this, [this](int){
        ui->pbGraphApply->setEnabled(true);
    });

    connect(ui->sbWindowHeight, &QSpinBox::valueChanged, this, [this](int){
        ui->pbGraphApply->setEnabled(true);
    });

    connect(ui->cbLimitFPS, &QCheckBox::checkStateChanged, this, [this](Qt::CheckState state){

        ui->pbGraphApply->setEnabled(true);
        ui->sbMaxFPS->setEnabled(state == Qt::Checked);
    });

    connect(ui->sbMaxFPS, &QSpinBox::valueChanged, this, [this](int){
        ui->pbGraphApply->setEnabled(true);
    });

    connect(ui->sbViewDistance, &QSpinBox::valueChanged, this, [this](int){
        ui->pbGraphApply->setEnabled(true);
    });

    connect(ui->sbDisplayNumber, &QSpinBox::valueChanged, this, [this](int){
        ui->pbGraphApply->setEnabled(true);
    });

    connect(ui->hsMSAA, &QSlider::valueChanged, this, [this](int){
        ui->pbGraphApply->setEnabled(true);
        ui->lMSAA->setText(QString("%1x").arg((1 << ui->hsMSAA->value())));
    });

    loadGraphicsSettings("settings");
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

    // Выбираем самый производительный GPU из найденных в качестве текущего
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
    }    

    // Если видюха еще не настраивалась никогда
    if (current_gpu_idx == -1)
    {
        // выбираем лючшую из найденных
        ui->cbListGPU->setCurrentIndex(best_gpu_idx);

        applyGraphSettings(fd_list, ui);

        updateGraphSettings(fd_list, ui);

        // Сохраняем настройки
        saveGraphSettings(fd_list);
    }
    else
    {
        ui->cbListGPU->setCurrentIndex(current_gpu_idx);
    }

    connect(ui->cbListGPU, &QComboBox::currentIndexChanged,
            this, &GraphSettingsWindow::slotOnChangeCurrentGPU);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void GraphSettingsWindow::loadGraphicsSettings(QString file_name)
{
    FileSystem &fs = FileSystem::getInstance();
    QString config_dir = QString(fs.getConfigDir().c_str());

    settings_path = config_dir + fs.separator() + file_name + ".xml";

    QString secName = "Viewer";

    CfgReader   cfg;

    fd_list.clear();

    if (cfg.load(settings_path))
    {
        int width = 0;
        cfg.getInt(secName, WIDTH, width);
        fd_list.append(QPair<QString, QVariant>(WIDTH, width));

        int height = 0;
        cfg.getInt(secName, HEIGHT, height);
        fd_list.append(QPair<QString, QVariant>(HEIGHT, height));

        int fullscreen = 0;
        cfg.getInt(secName, FULLSCREEN, fullscreen);
        fd_list.append(QPair<QString, QVariant>(FULLSCREEN, fullscreen));

        double fovY = 0;
        cfg.getDouble(secName, FOV_Y, fovY);
        fd_list.append(QPair<QString, QVariant>(FOV_Y, fovY));

        double zNear = 0;
        cfg.getDouble(secName, ZNEAR, zNear);
        fd_list.append(QPair<QString, QVariant>(ZNEAR, zNear));

        double zFar = 0;
        cfg.getDouble(secName, ZFAR, zFar);
        fd_list.append(QPair<QString, QVariant>(ZFAR, zFar));

        int screen_num = 0;
        cfg.getInt(secName, SCREEN_NUM, screen_num);
        fd_list.append(QPair<QString, QVariant>(SCREEN_NUM, screen_num));

        int win_decor = 0;
        cfg.getInt(secName, WIN_DECOR, win_decor);
        fd_list.append(QPair<QString, QVariant>(WIN_DECOR, win_decor));

        int double_buff = 0;
        cfg.getInt(secName, DOUBLE_BUFF, double_buff);
        fd_list.append(QPair<QString, QVariant>(DOUBLE_BUFF, double_buff));

        int vsync = 0;
        cfg.getInt(secName, VSYNC, vsync);
        fd_list.append(QPair<QString, QVariant>(VSYNC, vsync));

        int max_fps = 0;
        cfg.getInt(secName, MAX_FPS, max_fps);
        fd_list.append(QPair<QString, QVariant>(MAX_FPS, max_fps));

        cfg.getInt(secName, PHYSICAL_DEVICE, current_gpu_idx);
        fd_list.append(QPair<QString, QVariant>(PHYSICAL_DEVICE, current_gpu_idx));

        double view_dist = 0;
        cfg.getDouble(secName, VIEW_DIST, view_dist);
        fd_list.append(QPair<QString, QVariant>(VIEW_DIST, view_dist));

        int samples = 0;
        cfg.getInt(secName, SAMPLES, samples);
        fd_list.append(QPair<QString, QVariant>(SAMPLES, samples));

        updateGraphSettings(fd_list, ui);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void GraphSettingsWindow::updateGraphSettings(FieldsDataList &fd_list, Ui::GraphSettingsWindow *ui)
{
    ui->sbWindowWidth->setValue(findSetting(WIDTH, fd_list).second.toInt());
    ui->sbWindowHeight->setValue(findSetting(HEIGHT, fd_list).second.toInt());

    findSetting(FULLSCREEN, fd_list).second == 1 ?
        ui->cbFullScreenMode->setCheckState(Qt::CheckState::Checked) :
        ui->cbFullScreenMode->setCheckState(Qt::CheckState::Unchecked);

    findSetting(WIN_DECOR, fd_list).second == 1 ?
        ui->cbWindowDecoration->setCheckState(Qt::CheckState::Checked) :
        ui->cbWindowDecoration->setCheckState(Qt::CheckState::Unchecked);

    findSetting(VSYNC, fd_list).second == 1 ?
        ui->cbLimitFPS->setCheckState(Qt::CheckState::Unchecked) :
        ui->cbLimitFPS->setCheckState(Qt::CheckState::Checked);

    ui->sbMaxFPS->setValue(findSetting(MAX_FPS, fd_list).second.toInt());

    ui->cbListGPU->setCurrentIndex(findSetting(PHYSICAL_DEVICE, fd_list).second.toInt());

    ui->sbDisplayNumber->setValue(findSetting(SCREEN_NUM, fd_list).second.toInt());

    ui->sbViewDistance->setValue(findSetting(VIEW_DIST, fd_list).second.toInt());

    int samples = findSetting(SAMPLES, fd_list).second.toInt();
    int s = qRound(std::log2(samples));
    ui->hsMSAA->setValue(s);

    ui->pbGraphApply->setEnabled(false);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void GraphSettingsWindow::applyGraphSettings(FieldsDataList &fd_list,
                                             Ui::GraphSettingsWindow *ui)
{
    int idx = 0;

    findSetting(WIDTH, fd_list, idx);
    fd_list[idx] = QPair<QString, QVariant>(WIDTH, ui->sbWindowWidth->value());

    findSetting(HEIGHT, fd_list, idx);
    fd_list[idx] = QPair<QString, QVariant>(HEIGHT, ui->sbWindowHeight->value());

    findSetting(FULLSCREEN, fd_list, idx);
    if (ui->cbFullScreenMode->checkState() == Qt::CheckState::Checked)
    {
        fd_list[idx] = QPair<QString, QVariant>(FULLSCREEN, 1);
    }
    else
    {
        fd_list[idx] = QPair<QString, QVariant>(FULLSCREEN, 0);
    }

    findSetting(VSYNC, fd_list, idx);
    if (ui->cbLimitFPS->checkState() == Qt::CheckState::Checked)
    {
        fd_list[idx] = QPair<QString, QVariant>(VSYNC, 0);
    }
    else
    {
        fd_list[idx] = QPair<QString, QVariant>(VSYNC, 1);
    }

    findSetting(MAX_FPS, fd_list, idx);
    fd_list[idx] = QPair<QString, QVariant>(MAX_FPS, ui->sbMaxFPS->value());

    findSetting(PHYSICAL_DEVICE, fd_list, idx);
    fd_list[idx] = QPair<QString, QVariant>(PHYSICAL_DEVICE, ui->cbListGPU->currentIndex());

    findSetting(WIN_DECOR, fd_list, idx);
    if (ui->cbWindowDecoration->checkState() == Qt::CheckState::Checked)
    {
        fd_list[idx] = QPair<QString, QVariant>(WIN_DECOR, 1);
    }
    else
    {
        fd_list[idx] = QPair<QString, QVariant>(WIN_DECOR, 0);
    }

    findSetting(SCREEN_NUM, fd_list, idx);
    fd_list[idx] = QPair<QString, QVariant>(SCREEN_NUM, ui->sbDisplayNumber->value());

    findSetting(VIEW_DIST, fd_list, idx);
    fd_list[idx] = QPair<QString, QVariant>(VIEW_DIST, ui->sbViewDistance->value());

    findSetting(SAMPLES, fd_list, idx);
    int samples = (1 << ui->hsMSAA->value());
    fd_list[idx] = QPair<QString, QVariant>(SAMPLES, samples);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void GraphSettingsWindow::saveGraphSettings(FieldsDataList &fd_list)
{
    CfgEditor editor;

    editor.editFile(settings_path, "Viewer", fd_list);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void GraphSettingsWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);

    loadGraphicsSettings("settings");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void GraphSettingsWindow::slotOnChangeCurrentGPU(int idx)
{
    ui->pbGraphApply->setEnabled(true);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void GraphSettingsWindow::slotApplySettings()
{
    applyGraphSettings(fd_list, ui);

    updateGraphSettings(fd_list, ui);

    ui->pbGraphApply->setEnabled(false);

    saveGraphSettings(fd_list);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void GraphSettingsWindow::slotCancelSettings()
{
    updateGraphSettings(fd_list, ui);
    ui->pbGraphApply->setEnabled(false);
    this->hide();
}
