#ifndef     SPEEDMAP_H
#define     SPEEDMAP_H

#include    "device.h"

//------------------------------------------------------------------------------
// Устройство для получения ограничений скорости из топологии
//------------------------------------------------------------------------------
class DEVICE_EXPORT SpeedMap : public Device
{
public:

    /// Конструктор
    SpeedMap(QObject *parent = Q_NULLPTR);

    /// Деструктор
    ~SpeedMap();

    /// Задать направление поиска следующего ограничения:
    /// 1 - вперёд по траектории, -1 - назад  по траектории
    void setDirection(int direction);

    /// Задать дистанцию для поиска минимального текущего ограничения (длину поезда), м
    void setCurrentSearchDistance(double distance);

    /// Задать дистанцию поиска следующего ограничения, м
    void setNextSearchDistance(double distance);

    /// Текущее ограничение скорости, км/ч
    double getCurrentLimit() const;

    /// Следующее ограничение скорости, км/ч
    double getNextLimit() const;

    /// Расстояние до следующего ограничения скорости, м
    double getNextLimitDistance() const;

    virtual void step(double t, double dt);

    enum
    {
        SIZE_OF_OUTPUTS = 4,        ///< Размер массива исходящих сигналов

        OUTPUT_SEARCH_DIRECTION = 0,    ///< Направление поиска следующего ограничения
        OUTPUT_CUR_SEARCH_DISTANCE = 1, ///< Дистанция поиска текущего ограничения, м
        OUTPUT_NEXT_SEARCH_DISTANCE = 2,///< Дистанция поиска следующего ограничения, м
        OUTPUT_BRAKE_ACCELERATION = 3,  ///< Ускорение торможения, м/с^2

        SIZE_OF_INPUTS = 3,         ///< Размер массива входящих сигналов

        INPUT_CURRENT_LIMIT = 0,    ///< Текущее ограничение скорости, км/ч
        INPUT_NEXT_LIMIT = 1,       ///< Следующее ограничение скорости, км/ч
        INPUT_NEXT_DISTANCE = 2     ///< Расстояние до следующего ограничения скорости, м
    };

private:

    virtual void ode_system(const state_vector_t &Y,
                            state_vector_t &dYdt,
                            double t);

    /// Загрузка параметров из конфигурационного файла
    virtual void load_config(CfgReader &cfg);

};

#endif // SPEEDMAP_H
