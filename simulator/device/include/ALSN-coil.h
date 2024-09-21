#ifndef     COIL_ALSN_H
#define     COIL_ALSN_H

#include    "device.h"
#include    "ALSN-struct.h"

//------------------------------------------------------------------------------
// Устройство для получения частотного кода из рельсовых цепей АЛСН
//------------------------------------------------------------------------------
class DEVICE_EXPORT CoilALSN : public Device
{
public:

    /// Конструктор
    CoilALSN(QObject *parent = Q_NULLPTR);

    /// Деструктор
    ~CoilALSN();

    /// Задать направление:
    /// 1 - вперёд по траектории, -1 - назад  по траектории
    void setDirection(int direction);

    /// Несущая частота сигнала, Гц
    double getFrequency() const;

    /// Кодовый сигнал
    ALSN getCode() const;

    /// Расстояние до следующего светофора, м
    double getNextSignalDistance() const;

    /// Литер следующего светофора
    QString getNextSignalLiter() const;

    virtual void step(double t, double dt);

    enum
    {
        SIZE_OF_OUTPUTS = 1,    ///< Размер массива исходящих сигналов

        OUTPUT_DIRECTION = 0,   ///< Направление

        SIZE_OF_INPUTS = 14,    ///< Размер массива входящих сигналов

        INPUT_FREQUENCY = 0,    ///< Несущая частота сигнала, Гц
        INPUT_CODE = 1,         ///< Кодовый сигнал
        INPUT_NEXT_DISTANCE = 2,///< Расстояние до следующего светофора, м

        INPUT_LITER_SIZE = 3,   ///< Размер массива символов литера светофора
        INPUT_LITER_BEGIN = 4,  ///< Начало массива символов литера светофора
        INPUT_LITER_MAX_SIZE = SIZE_OF_INPUTS - INPUT_LITER_BEGIN
    };

private:

    QString next_liter = "";

    virtual void ode_system(const state_vector_t &Y,
                            state_vector_t &dYdt,
                            double t);

    /// Загрузка параметров из конфигурационного файла
    virtual void load_config(CfgReader &cfg);

};

#endif // COIL_ALSN_H
