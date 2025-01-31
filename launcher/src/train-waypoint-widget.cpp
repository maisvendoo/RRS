#include    "train-waypoint-widget.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TrainWaypointWidget::TrainWaypointWidget(std::vector<train_info_t>       *trains_info,
                                         std::vector<trajectory_info_t>  *trajectrories,
                                         std::vector<train_position_t>   *fwd_train_positions,
                                         std::vector<train_position_t>   *bwd_train_positions,
                                         QWidget *parent) : QWidget(parent)
    , trains_info(trains_info)
    , trajectrories(trajectrories)
    , fwd_train_positions(fwd_train_positions)
    , bwd_train_positions(bwd_train_positions)
{
    vblLines = new QVBoxLayout(this);
    hblLine1 = new QHBoxLayout();
    hblLine2 = new QHBoxLayout();

    cbTrainConfigSelect = new QComboBox(this);
    cbWaypointDirectionSelect = new QComboBox(this);
    cbWaypointSelect = new QComboBox(this);

    hblLine1->addWidget(cbTrainConfigSelect);
    hblLine1->addWidget(cbWaypointDirectionSelect);
    hblLine1->addWidget(cbWaypointSelect);
    vblLines->addLayout(hblLine1);

    cbTrajectoryNameSelect = new QComboBox(this);
    cbTrajectoryDirectionSelect = new QComboBox(this);
    dsbTrajectoryCoordinate = new QDoubleSpinBox(this);
    dsbTrajectoryCoordinate->setMaximum(40000000.0);
    dsbTrajectoryCoordinate->setDecimals(2);

    hblLine2->addWidget(cbTrajectoryNameSelect);
    hblLine2->addWidget(cbTrajectoryDirectionSelect);
    hblLine2->addWidget(dsbTrajectoryCoordinate);
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
    delete dsbTrajectoryCoordinate;
    delete cbTrajectoryDirectionSelect;
    delete cbTrajectoryNameSelect;

    delete cbWaypointSelect;
    delete cbWaypointDirectionSelect;
    delete cbTrainConfigSelect;

    delete hblLine2;
    delete hblLine1;
    delete vblLines;
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
    (void) train_idx;
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
    (void) traj_idx;

    int waypoint_idx = cbWaypointSelect->currentIndex();
    train_position_t tp;
    if (!getCurrentWaypoint(waypoint_idx, tp))
        return;

    if (cbTrajectoryNameSelect->currentText() != tp.trajectory_name)
    {
        cbWaypointSelect->setCurrentIndex(0);
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
    if (abs(traj_coord - tp.traj_coord) > 1.0)
    {
        cbWaypointSelect->setCurrentIndex(0);
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
        return;
    }

    cbTrajectoryNameSelect->setCurrentIndex(traj_idx);
    cbTrajectoryDirectionSelect->setCurrentIndex((tp.direction > 0) ? 0 : 1);
    dsbTrajectoryCoordinate->setValue(tp.traj_coord);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrainWaypointWidget::resetTrajectorySelectWidgets()
{
    cbTrajectoryNameSelect->setCurrentIndex(0);
    cbTrajectoryDirectionSelect->setCurrentIndex(cbWaypointDirectionSelect->currentIndex());
    dsbTrajectoryCoordinate->setValue(0.0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool TrainWaypointWidget::getCurrentWaypoint(int waypoint_idx, train_position_t &tp)
{
    if (waypoint_idx <= 0)
        return false;

    int dir_idx = cbWaypointDirectionSelect->currentIndex();
    std::vector<train_position_t> *train_positions =
        (dir_idx == 0) ?
            fwd_train_positions :
            bwd_train_positions;

    if (waypoint_idx > train_positions->size())
    {
        cbWaypointSelect->setCurrentIndex(0);
        return false;
    }

    tp = train_positions->at(waypoint_idx - 1);
    return true;
}
