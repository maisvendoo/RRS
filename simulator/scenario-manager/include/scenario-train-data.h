#ifndef     SCENARIO_TRAIN_DATA_H
#define     SCENARIO_TRAIN_DATA_H

#include    <string>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct scenario_train_data_t
{
    /// Имя файла поезда
    std::string train_file;
    /// Имя траектории
    std::string traj_name;
    /// Координата на траектории
    double traj_coord;
    /// Направление
    int direction;
};

#endif
