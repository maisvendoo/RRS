#include    <qabstractitemview.h>
#include    "train-waypoint-widget.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TrainWaypointWidget::TrainWaypointWidget(std::vector<train_info_t>       *trains_info,
                                         std::vector<trajectory_info_t>  *trajectrories,
                                         std::vector<train_position_t>   *fwd_train_positions,
                                         std::vector<train_position_t>   *bwd_train_positions,
                                         QIcon *icon_ok,
                                         QIcon *icon_cancel,
                                         QIcon *icon_warn,
                                         QWidget *parent) : QFrame(parent)
    , trains_info(trains_info)
    , trajectrories(trajectrories)
    , fwd_train_positions(fwd_train_positions)
    , bwd_train_positions(bwd_train_positions)
    , icon_ok(icon_ok)
    , icon_cancel(icon_cancel)
    , icon_warn(icon_warn)
{
    setFrameStyle(QFrame::Panel | QFrame::Sunken);

    // Структура расположения элементов
    vblLines = new QVBoxLayout(this);

    hblLine1 = new QHBoxLayout();
    hblLine2 = new QHBoxLayout();

    vblTrain = new QVBoxLayout(this);
    vblWaypoint = new QVBoxLayout(this);
    vblTrajpoint = new QVBoxLayout(this);

    hblTrainHeader = new QHBoxLayout();
    hblTrainConfig = new QHBoxLayout();
    hblWaypointHeader = new QHBoxLayout();
    hblWaypointConfig = new QHBoxLayout();
    hblTrajpointHeader = new QHBoxLayout();
    hblTrajpointConfig = new QHBoxLayout();

    // Заголовок к настройке поезда
    lTrainHeaderIcon = new QLabel(this);
    lTrainHeaderIcon->setPixmap(icon_cancel->pixmap(16));
    lTrainHeaderText = new QLabel(this);
    lTrainHeaderText->setText(tr("Train selection:"));
    sTrainHeaderRight = new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding);

    hblTrainHeader->addWidget(lTrainHeaderIcon);
    hblTrainHeader->addWidget(lTrainHeaderText);
    hblTrainHeader->addItem(sTrainHeaderRight);
    vblTrain->addLayout(hblTrainHeader);

    // Настройка поезда
    cbTrainConfigSelect = new QComboBox(this);

    hblTrainConfig->addWidget(cbTrainConfigSelect);
    vblTrain->addLayout(hblTrainConfig);
    hblLine1->addLayout(vblTrain);

    // Заголовок к настройке предустановленных стартовых точек
    lWaypointHeaderIcon = new QLabel(this);
    lWaypointHeaderIcon->setPixmap(icon_warn->pixmap(16));
    lWaypointHeaderText = new QLabel(this);
    lWaypointHeaderText->setText(tr("Route's predefined start waypoints:"));
    sWaypointHeaderRight = new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding);

    hblWaypointHeader->addWidget(lWaypointHeaderIcon);
    hblWaypointHeader->addWidget(lWaypointHeaderText);
    hblWaypointHeader->addItem(sWaypointHeaderRight);
    vblWaypoint->addLayout(hblWaypointHeader);

    // Настройка предустановленных стартовых точек
    cbWaypointDirectionSelect = new QComboBox(this);
    cbWaypointDirectionSelect->setFixedWidth(80);
    cbWaypointSelect = new QComboBox(this);

    hblWaypointConfig->addWidget(cbWaypointDirectionSelect);
    hblWaypointConfig->addWidget(cbWaypointSelect);
    vblWaypoint->addLayout(hblWaypointConfig);
    hblLine1->addLayout(vblWaypoint);
    vblLines->addLayout(hblLine1);

    // Заголовок к настройке стартовой точки на траектории
    lTrajectoryHeaderIcon = new QLabel(this);
    lTrajectoryHeaderIcon->setPixmap(icon_cancel->pixmap(16));
    lTrajectoryHeaderText = new QLabel(this);
    lTrajectoryHeaderText->setText(tr("Configuration of the start point:"));
    sTrajectoryHeaderRight = new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding);

    hblTrajpointHeader->addWidget(lTrajectoryHeaderIcon);
    hblTrajpointHeader->addWidget(lTrajectoryHeaderText);
    hblTrajpointHeader->addItem(sTrajectoryHeaderRight);
    vblTrajpoint->addLayout(hblTrajpointHeader);

    // Настройка стартовой точки на траектории
    cbTrajectoryNameSelect = new QComboBox(this);
    cbTrajectoryDirectionSelect = new QComboBox(this);
    cbTrajectoryDirectionSelect->setFixedWidth(80);
    lTrajectoryCoordinateText = new QLabel(this);
    lTrajectoryCoordinateText->setText(tr("Trajectory coordinate:"));
    lTrajectoryCoordinateText->setFixedWidth(120);
    dsbTrajectoryCoordinate = new QDoubleSpinBox(this);
    dsbTrajectoryCoordinate->setFixedWidth(80);
    dsbTrajectoryCoordinate->setMaximum(40000000.0);
    dsbTrajectoryCoordinate->setDecimals(2);

    hblTrajpointConfig->addWidget(cbTrajectoryNameSelect);
    hblTrajpointConfig->addWidget(cbTrajectoryDirectionSelect);
    hblTrajpointConfig->addWidget(lTrajectoryCoordinateText);
    hblTrajpointConfig->addWidget(dsbTrajectoryCoordinate);
    vblTrajpoint->addLayout(hblTrajpointConfig);
    hblLine2->addLayout(vblTrajpoint);
    vblLines->addLayout(hblLine2);

    setDirectionSelectWidget();
    setTrainSelectWidget();
    setWaypointSelectWidget();
    setTrajectorySelectWidget();

    connect(cbTrainConfigSelect, &QComboBox::currentIndexChanged,
            this, &TrainWaypointWidget::slotTrainConfigChange);

    connect(cbWaypointDirectionSelect, &QComboBox::currentIndexChanged,
            this, &TrainWaypointWidget::slotWaypointDirectionChange);

    connect(cbWaypointSelect, &QComboBox::currentIndexChanged,
            this, &TrainWaypointWidget::slotWaypointChange);

    connect(cbTrajectoryNameSelect, &QComboBox::currentIndexChanged,
            this, &TrainWaypointWidget::slotTrajectoryNameChange);

    connect(cbTrajectoryDirectionSelect, &QComboBox::currentIndexChanged,
            this, &TrainWaypointWidget::slotTrajectoryDirectionChange);

    connect(dsbTrajectoryCoordinate, &QDoubleSpinBox::valueChanged,
            this, &TrainWaypointWidget::slotTrajectoryCoordinateChange);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TrainWaypointWidget::~TrainWaypointWidget()
{
    disconnect(cbTrainConfigSelect, &QComboBox::currentIndexChanged,
               this, &TrainWaypointWidget::slotTrainConfigChange);

    disconnect(cbWaypointDirectionSelect, &QComboBox::currentIndexChanged,
               this, &TrainWaypointWidget::slotWaypointDirectionChange);

    disconnect(cbWaypointSelect, &QComboBox::currentIndexChanged,
               this, &TrainWaypointWidget::slotWaypointChange);

    disconnect(cbTrajectoryNameSelect, &QComboBox::currentIndexChanged,
               this, &TrainWaypointWidget::slotTrajectoryNameChange);

    disconnect(cbTrajectoryDirectionSelect, &QComboBox::currentIndexChanged,
               this, &TrainWaypointWidget::slotTrajectoryDirectionChange);

    disconnect(dsbTrajectoryCoordinate, &QDoubleSpinBox::valueChanged,
               this, &TrainWaypointWidget::slotTrajectoryCoordinateChange);
/*
    delete dsbTrajectoryCoordinate;
    delete cbTrajectoryDirectionSelect;
    delete cbTrajectoryNameSelect;

    delete sLine2headerRight;
    delete lTrajectoryPointSelectText;
    delete lTrajectoryPointSelectIcon;

    delete cbWaypointSelect;
    delete cbWaypointDirectionSelect;
    delete cbTrainConfigSelect;

    delete sLine1headerRight;
    delete lWaypointSelectText;
    delete lWaypointSelectIcon;
    delete sLine1headerMiddle;
    delete lTrainConfigSelectText;
    delete lTrainConfigSelectIcon;

    delete hblLine2content;
    delete hblLine2header;
    delete hblLine1content;
    delete hblLine1header;
    delete vblLines;
*/
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString TrainWaypointWidget::getTrainName()
{
    int train_idx = cbTrainConfigSelect->currentIndex();
    if ((train_idx <= 0) || (train_idx > trains_info->size()))
        return QString("<Not Selected>");

    return trains_info->at(train_idx - 1).train_title;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
active_train_t TrainWaypointWidget::getActiveTrainConfig()
{
    active_train_t at;

    int train_idx = cbTrainConfigSelect->currentIndex();
    if ((train_idx <= 0) || (train_idx > trains_info->size()))
    {
        at.is_active = false;
        return at;
    }
    at.train_info = trains_info->at(train_idx - 1);

    int traj_idx = cbTrajectoryNameSelect->currentIndex();
    if ((traj_idx <= 0) || (traj_idx > trajectrories->size()))
    {
        at.is_active = false;
        return at;
    }
    at.train_position.trajectory_name = trajectrories->at(traj_idx - 1).name;

    int dir_idx = cbTrajectoryDirectionSelect->currentIndex();
    at.train_position.direction = (dir_idx == 0) ? 1 : -1;

    at.train_position.traj_coord = dsbTrajectoryCoordinate->value();

    at.train_position.name = cbWaypointSelect->currentText();

    at.is_active = true;

    return at;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrainWaypointWidget::slotTrainConfigChange(int train_idx)
{
    bool is_selected = (train_idx > 0) && (train_idx <= trains_info->size());

    if (is_train_config_selected != is_selected)
    {
        is_train_config_selected = is_selected;

        if (is_train_config_selected)
            lTrainHeaderIcon->setPixmap(icon_ok->pixmap(16));
        else
            lTrainHeaderIcon->setPixmap(icon_cancel->pixmap(16));
    }

    emit activeTrainChanged();
    emit trainConfigChanged();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrainWaypointWidget::slotWaypointDirectionChange(int dir_idx)
{
    (void) dir_idx;
    setWaypointSelectWidget();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrainWaypointWidget::slotWaypointChange(int waypoint_idx)
{
    train_position_t tp;
    if (!getCurrentWaypoint(waypoint_idx, tp))
        return;

    setTrajectorySelectWidgets(tp);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrainWaypointWidget::slotTrajectoryNameChange(int traj_idx)
{
    bool is_selected = (traj_idx > 0) && (traj_idx <= trajectrories->size());
    if (is_trajectory_selected != is_selected)
    {
        is_trajectory_selected = is_selected;

        if (is_trajectory_selected)
            lTrajectoryHeaderIcon->setPixmap(icon_ok->pixmap(16));
        else
            lTrajectoryHeaderIcon->setPixmap(icon_cancel->pixmap(16));
    }

    emit activeTrainChanged();

    int waypoint_idx = cbWaypointSelect->currentIndex();
    train_position_t tp;
    if (!getCurrentWaypoint(waypoint_idx, tp))
        return;

    if (cbTrajectoryNameSelect->currentText() != tp.trajectory_name)
    {
        cbWaypointSelect->setCurrentIndex(0);
        lWaypointHeaderIcon->setPixmap(icon_warn->pixmap(16));
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrainWaypointWidget::slotTrajectoryDirectionChange(int dir_idx)
{
    (void) dir_idx;

    int waypoint_idx = cbWaypointSelect->currentIndex();
    train_position_t tp;
    if (!getCurrentWaypoint(waypoint_idx, tp))
        return;

    dir_idx = cbTrajectoryDirectionSelect->currentIndex();
    if (dir_idx != ((tp.direction > 0) ? 0 : 1))
    {
        cbWaypointSelect->setCurrentIndex(0);
        lWaypointHeaderIcon->setPixmap(icon_warn->pixmap(16));
    }

    emit activeTrainChanged();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrainWaypointWidget::slotTrajectoryCoordinateChange(double coord)
{
    (void) coord;

    int waypoint_idx = cbWaypointSelect->currentIndex();
    train_position_t tp;
    if (!getCurrentWaypoint(waypoint_idx, tp))
        return;

    double traj_coord = dsbTrajectoryCoordinate->value();
    if (abs(traj_coord - tp.traj_coord) > 0.5)
    {
        cbWaypointSelect->setCurrentIndex(0);
        lWaypointHeaderIcon->setPixmap(icon_warn->pixmap(16));
    }

    emit activeTrainChanged();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrainWaypointWidget::setDirectionSelectWidget()
{
    cbWaypointDirectionSelect->clear();
    cbWaypointDirectionSelect->addItem(tr("Forward"));
    cbWaypointDirectionSelect->addItem(tr("Backward"));
    cbWaypointDirectionSelect->setCurrentIndex(0);

    cbTrajectoryDirectionSelect->clear();
    cbTrajectoryDirectionSelect->addItem(tr("Forward"));
    cbTrajectoryDirectionSelect->addItem(tr("Backward"));
    cbTrajectoryDirectionSelect->setCurrentIndex(0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrainWaypointWidget::setTrainSelectWidget()
{
    cbTrainConfigSelect->clear();
    cbTrainConfigSelect->addItem(tr("<Not selected>"));
    for (auto train : (*trains_info))
    {
        cbTrainConfigSelect->addItem(train.train_title);
    }
    cbTrainConfigSelect->setCurrentIndex(0);
    lTrainHeaderIcon->setPixmap(icon_cancel->pixmap(16));

    if (is_train_config_selected)
    {
        is_train_config_selected = false;
        emit activeTrainChanged();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrainWaypointWidget::setWaypointSelectWidget()
{
    cbWaypointSelect->clear();
    cbWaypointSelect->addItem(tr("<Not selected>"));

    if (cbWaypointDirectionSelect->currentIndex() == 0)
    {
        for (auto waypoint : (*fwd_train_positions))
        {
            cbWaypointSelect->addItem(waypoint.name);
        }
    }
    else
    {
        for (auto waypoint : (*bwd_train_positions))
        {
            cbWaypointSelect->addItem(waypoint.name);
        }
    }
    cbWaypointSelect->setCurrentIndex(0);
    lWaypointHeaderIcon->setPixmap(icon_warn->pixmap(16));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrainWaypointWidget::setTrajectorySelectWidget()
{
    cbTrajectoryNameSelect->clear();
    cbTrajectoryNameSelect->addItem(tr("<Not selected>"));

    for (auto traj : (*trajectrories))
    {
        cbTrajectoryNameSelect->addItem(traj.name);
    }
    cbTrajectoryNameSelect->setCurrentIndex(0);
    lTrajectoryHeaderIcon->setPixmap(icon_cancel->pixmap(16));

    if (is_trajectory_selected)
    {
        is_trajectory_selected = false;
        emit activeTrainChanged();
    }

    dsbTrajectoryCoordinate->setValue(0.0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrainWaypointWidget::setTrajectorySelectWidgets(train_position_t tp)
{
    int traj_idx = cbTrajectoryNameSelect->findText(tp.trajectory_name);
    if (traj_idx == -1)
    {
        cbTrajectoryNameSelect->setCurrentIndex(0);
        lWaypointHeaderIcon->setPixmap(icon_warn->pixmap(16));

        if (is_trajectory_selected)
        {
            is_trajectory_selected = false;
            emit activeTrainChanged();
        }
        return;
    }

    cbTrajectoryNameSelect->setCurrentIndex(traj_idx);
    cbTrajectoryDirectionSelect->setCurrentIndex((tp.direction > 0) ? 0 : 1);
    dsbTrajectoryCoordinate->setValue(tp.traj_coord);

    lWaypointHeaderIcon->setPixmap(icon_ok->pixmap(16));

    if (!is_trajectory_selected)
    {
        is_trajectory_selected = true;
        lTrajectoryHeaderIcon->setPixmap(icon_ok->pixmap(16));
        emit activeTrainChanged();
    }

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrainWaypointWidget::resetTrajectorySelectWidgets()
{
    cbTrajectoryNameSelect->setCurrentIndex(0);
    cbTrajectoryDirectionSelect->setCurrentIndex(cbWaypointDirectionSelect->currentIndex());
    dsbTrajectoryCoordinate->setValue(0.0);

    if (is_trajectory_selected)
    {
        is_trajectory_selected = false;
        lTrajectoryHeaderIcon->setPixmap(icon_cancel->pixmap(16));
        emit activeTrainChanged();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool TrainWaypointWidget::getCurrentWaypoint(int waypoint_idx, train_position_t &tp)
{
    if (waypoint_idx <= 0)
    {
        lWaypointHeaderIcon->setPixmap(icon_warn->pixmap(16));
        return false;
    }

    int dir_idx = cbWaypointDirectionSelect->currentIndex();
    std::vector<train_position_t> *train_positions =
        (dir_idx == 0) ?
            fwd_train_positions :
            bwd_train_positions;

    if (waypoint_idx > train_positions->size())
    {
        cbWaypointSelect->setCurrentIndex(0);
        lWaypointHeaderIcon->setPixmap(icon_warn->pixmap(16));
        return false;
    }

    tp = train_positions->at(waypoint_idx - 1);
    return true;
}
