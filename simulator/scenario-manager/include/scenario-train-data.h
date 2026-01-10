#ifndef     SCENARIO_TRAIN_DATA_H
#define     SCENARIO_TRAIN_DATA_H

#include    <string>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct scenario_train_data_t
{
    /// Имя поезда, для последующей его идентификации
    std::string name = "";
    /// Имя конфига поезда
    std::string train_config;
    /// Имя траектории
    std::string traj_name;
    /// Координата на траектории
    double traj_coord;
    /// Направление
    int direction;

    void setIndex(size_t idx)
    {
        train_idx = static_cast<int>(idx);
    }

    int getIndex() const
    {
        return train_idx;
    }

private:

    /// Индекс поезда умышленно знаковый (пока не знаю зачем)
    int train_idx = -1;
};

#endif
