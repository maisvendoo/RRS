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

    vblLines = new QVBoxLayout(this);
    hblLine1header = new QHBoxLayout();
    hblLine1content = new QHBoxLayout();
    hblLine2header = new QHBoxLayout();
    hblLine2content = new QHBoxLayout();

    lTrainConfigSelectIcon = new QLabel(this);
    lTrainConfigSelectIcon->setPixmap(icon_cancel->pixmap(16));
    lTrainConfigSelectText = new QLabel(this);
    lTrainConfigSelectText->setText(tr("Train selection:"));
    sLine1headerMiddle = new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding);
    lWaypointSelectIcon = new QLabel(this);
    lWaypointSelectIcon->setPixmap(icon_warn->pixmap(16));
    lWaypointSelectText = new QLabel(this);
    lWaypointSelectText->setText(tr("Route's predefined start waypoints:"));
    sLine1headerRight = new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding);

    hblLine1header->addWidget(lTrainConfigSelectIcon);
    hblLine1header->addWidget(lTrainConfigSelectText);
    hblLine1header->addItem(sLine1headerMiddle);
    hblLine1header->addWidget(lWaypointSelectIcon);
    hblLine1header->addWidget(lWaypointSelectText);
    hblLine1header->addItem(sLine1headerRight);
    vblLines->addLayout(hblLine1header);

    cbTrainConfigSelect = new QComboBox(this);
    cbWaypointDirectionSelect = new QComboBox(this);
    cbWaypointDirectionSelect->setFixedWidth(80);
    cbWaypointSelect = new QComboBox(this);

    hblLine1content->addWidget(cbTrainConfigSelect);
    hblLine1content->addWidget(cbWaypointDirectionSelect);
    hblLine1content->addWidget(cbWaypointSelect);
    vblLines->addLayout(hblLine1content);

    lTrajectoryPointSelectIcon = new QLabel(this);
    lTrajectoryPointSelectIcon->setPixmap(icon_cancel->pixmap(16));
    lTrajectoryPointSelectText = new QLabel(this);
    lTrajectoryPointSelectText->setText(tr("Configuration of the start point:"));
    sLine2headerRight = new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding);

    hblLine2header->addWidget(lTrajectoryPointSelectIcon);
    hblLine2header->addWidget(lTrajectoryPointSelectText);
    hblLine2header->addItem(sLine2headerRight);
    vblLines->addLayout(hblLine2header);

    cbTrajectoryNameSelect = new QComboBox(this);
    cbTrajectoryDirectionSelect = new QComboBox(this);
    cbTrajectoryDirectionSelect->setFixedWidth(80);
    dsbTrajectoryCoordinate = new QDoubleSpinBox(this);
    dsbTrajectoryCoordinate->setMaximum(40000000.0);
    dsbTrajectoryCoordinate->setDecimals(2);

    hblLine2content->addWidget(cbTrajectoryNameSelect);
    hblLine2content->addWidget(cbTrajectoryDirectionSelect);
    hblLine2content->addWidget(dsbTrajectoryCoordinate);
    vblLines->addLayout(hblLine2content);

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
            lTrainConfigSelectIcon->setPixmap(icon_ok->pixmap(16));
        else
            lTrainConfigSelectIcon->setPixmap(icon_cancel->pixmap(16));

        emit activeTrainChanged();
    }

    emit trainConfigChanged(getTrainName());
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
            lTrajectoryPointSelectIcon->setPixmap(icon_ok->pixmap(16));
        else
            lTrajectoryPointSelectIcon->setPixmap(icon_cancel->pixmap(16));

        emit activeTrainChanged();
    }

    int waypoint_idx = cbWaypointSelect->currentIndex();
    train_position_t tp;
    if (!getCurrentWaypoint(waypoint_idx, tp))
        return;

    if (cbTrajectoryNameSelect->currentText() != tp.trajectory_name)
    {
        cbWaypointSelect->setCurrentIndex(0);
        lWaypointSelectIcon->setPixmap(icon_warn->pixmap(16));
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
        lWaypointSelectIcon->setPixmap(icon_warn->pixmap(16));
    }
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
        lWaypointSelectIcon->setPixmap(icon_warn->pixmap(16));
    }
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
    lTrainConfigSelectIcon->setPixmap(icon_cancel->pixmap(16));

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
    lWaypointSelectIcon->setPixmap(icon_warn->pixmap(16));
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
    lTrajectoryPointSelectIcon->setPixmap(icon_cancel->pixmap(16));

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
        lWaypointSelectIcon->setPixmap(icon_warn->pixmap(16));

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

    lWaypointSelectIcon->setPixmap(icon_ok->pixmap(16));

    if (!is_trajectory_selected)
    {
        is_trajectory_selected = true;
        lTrajectoryPointSelectIcon->setPixmap(icon_ok->pixmap(16));
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
        lTrajectoryPointSelectIcon->setPixmap(icon_cancel->pixmap(16));
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
        lWaypointSelectIcon->setPixmap(icon_warn->pixmap(16));
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
        lWaypointSelectIcon->setPixmap(icon_warn->pixmap(16));
        return false;
    }

    tp = train_positions->at(waypoint_idx - 1);
    return true;
}
