#ifndef     TRAINWAYPOINTWIDGET_H
#define     TRAINWAYPOINTWIDGET_H

#include    <QFrame>
#include    <QBoxLayout>
#include    <QSpacerItem>
#include    <QLabel>
#include    <QComboBox>
#include    <QDoubleSpinBox>
#include    <QCheckBox>

#include    <train-info.h>
#include    <trajectory-info.h>
#include    <waypoint.h>
#include    <active-train.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class TrainWaypointWidget : public QFrame
{
    Q_OBJECT

public:

    TrainWaypointWidget(std::vector<train_info_t>       *trains_info,
                        std::vector<trajectory_info_t>  *trajectrories,
                        std::vector<train_position_t>   *fwd_train_positions,
                        std::vector<train_position_t>   *bwd_train_positions,
                        QIcon *icon_ok,
                        QIcon *icon_cancel,
                        QIcon *icon_warn,
                        QWidget *parent = nullptr);

    ~TrainWaypointWidget();

    // Структура расположения элементов
    QVBoxLayout *vblLines;

    QHBoxLayout *hblLine1;
    QHBoxLayout *hblLine2;

    QVBoxLayout *vblTrain;
    QVBoxLayout *vblWaypoint;
    QVBoxLayout *vblTrajpoint;

    QHBoxLayout *hblTrainHeader;
    QHBoxLayout *hblTrainConfig;    
    QHBoxLayout *hblWaypointHeader;
    QHBoxLayout *hblWaypointConfig;
    QHBoxLayout *hblTrajpointHeader;
    QHBoxLayout *hblTrajpointConfig;

    // Заголовок и настройка поезда
    QLabel      *lTrainHeaderIcon;
    QLabel      *lTrainHeaderText;
    QSpacerItem *sTrainHeaderRight;
    QComboBox   *cbTrainConfigSelect;
    QCheckBox   *ckbAutopilot;

    // Заголовок и настройка предустановленных стартовых точек
    QLabel      *lWaypointHeaderIcon;
    QLabel      *lWaypointHeaderText;
    QSpacerItem *sWaypointHeaderRight;
    QComboBox   *cbWaypointDirectionSelect;
    QComboBox   *cbWaypointSelect;

    // Заголовок и настройка стартовой точки на траектории
    QLabel      *lTrajectoryHeaderIcon;
    QLabel      *lTrajectoryHeaderText;
    QSpacerItem *sTrajectoryHeaderRight;
    QComboBox   *cbTrajectoryNameSelect;
    QComboBox   *cbTrajectoryDirectionSelect;
    QLabel      *lTrajectoryCoordinateText;
    QDoubleSpinBox  *dsbTrajectoryCoordinate;

    // Информация о поездах и маршрутах
    std::vector<train_info_t>       *trains_info;
    std::vector<trajectory_info_t>  *trajectrories;
    std::vector<train_position_t>   *fwd_train_positions;
    std::vector<train_position_t>   *bwd_train_positions;

    // Иконки
    QIcon *icon_ok;
    QIcon *icon_cancel;
    QIcon *icon_warn;

    QString getTrainName();

    active_train_t getActiveTrainConfig();

signals:

    void trainConfigChanged();

    void activeTrainChanged(bool reset_start_config = true);

private slots:

    void slotTrainConfigChange(int train_idx);

    void slotWaypointDirectionChange(int dir_idx);

    void slotWaypointChange(int waypoint_idx);

    void slotTrajectoryNameChange(int traj_idx);

    void slotTrajectoryDirectionChange(int dir_idx);

    void slotTrajectoryCoordinateChange(double coord);

private:

    bool is_train_config_selected = false;
    bool is_trajectory_selected = false;

    void setDirectionSelectWidget();

    void setTrainSelectWidget();

    void setWaypointSelectWidget();

    void setTrajectorySelectWidget();

    void setTrajectorySelectWidgets(train_position_t tp);

    void resetTrajectorySelectWidgets();

    bool getCurrentWaypoint(int waypoint_idx, train_position_t &tp);
};

#endif // TRAINWAYPOINTWIDGET_H
