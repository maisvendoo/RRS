#ifndef     TRAINWAYPOINTWIDGET_H
#define     TRAINWAYPOINTWIDGET_H

#include    <QWidget>
#include    <QVBoxLayout>
#include    <QHBoxLayout>
#include    <QComboBox>
#include    <QDoubleSpinBox>

#include    <train-info.h>
#include    <trajectory-info.h>
#include    <waypoint.h>
#include    <active-train.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class TrainWaypointWidget : public QWidget
{
    Q_OBJECT

public:

    TrainWaypointWidget(std::vector<train_info_t>       *trains_info,
                        std::vector<trajectory_info_t>  *trajectrories,
                        std::vector<train_position_t>   *fwd_train_positions,
                        std::vector<train_position_t>   *bwd_train_positions,
                        QWidget *parent = Q_NULLPTR);

    ~TrainWaypointWidget();

    QVBoxLayout *vblLines;
    QHBoxLayout *hblLine1;
    QHBoxLayout *hblLine2;

    QComboBox   *cbTrainConfigSelect;
    QComboBox   *cbWaypointDirectionSelect;
    QComboBox   *cbWaypointSelect;

    QComboBox   *cbTrajectoryNameSelect;
    QComboBox   *cbTrajectoryDirectionSelect;
    QDoubleSpinBox  *dsbTrajectoryCoordinate;

    std::vector<train_info_t>       *trains_info;
    std::vector<trajectory_info_t>  *trajectrories;
    std::vector<train_position_t>   *fwd_train_positions;
    std::vector<train_position_t>   *bwd_train_positions;

    QString getTrainName();

    active_train_t getActiveTrainConfig();

signals:

    void trainConfigChanged(QString train_name);

private slots:

    void slotTrainConfigChange(int train_idx);

    void slotWaypointDirectionChange(int dir_idx);

    void slotWaypointChange(int waypoint_idx);

    void slotTrajectoryNameChange(int traj_idx);

    void slotTrajectoryDirectionChange(int dir_idx);

    void slotTrajectoryCoordinateChange(double coord);

private:

    void setDirectionSelectWidget();

    void setTrainSelectWidget();

    void setWaypointSelectWidget();

    void setTrajectorySelectWidget();

    void setTrajectorySelectWidgets(train_position_t tp);

    void resetTrajectorySelectWidgets();

    bool getCurrentWaypoint(int waypoint_idx, train_position_t &tp);
};

#endif // TRAINWAYPOINTWIDGET_H
