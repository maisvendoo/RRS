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

    hblLine2->addWidget(cbTrajectoryNameSelect);
    hblLine2->addWidget(cbTrajectoryDirectionSelect);
    hblLine2->addWidget(dsbTrajectoryCoordinate);
    vblLines->addLayout(hblLine2);

    setDirectionSelectWidget();
    setTrainSelectWidget();
    setWaypointSelectWidget();
    setTrajectorySelectWidget();
};

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

    return trains_info->at(train_idx).train_title;
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

    return at;
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
void TrainWaypointWidget::resetTrajectorySelectWidgets()
{
    cbTrajectoryNameSelect->setCurrentIndex(0);
    cbTrajectoryDirectionSelect->setCurrentIndex(cbWaypointDirectionSelect->currentIndex());
    dsbTrajectoryCoordinate->setValue(0.0);
}
