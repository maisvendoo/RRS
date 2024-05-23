#ifndef     PNEUMO_ANGLECOCK_H
#define     PNEUMO_ANGLECOCK_H

#include    "device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DEVICE_EXPORT PneumoAngleCock : public Device
{
public:

    /// Конструктор
    PneumoAngleCock(QObject *parent = Q_NULLPTR);

    /// Деструктор
    virtual ~PneumoAngleCock();

    /// Закрыть концевой кран
    void close();

    /// Открыть концевой кран
    void open();

    /// Cостояние концевого крана
    bool isOpened() const;

    /// Положение рукоятки: от 0.0 (кран закрыт) до 1.0 (кран открыт)
    double getHandlePosition() const;

    /// Задать объём трубы магистрали для вычисления максимального коэффициента перетока
    /// (для устойчивого расчёта не более 0.15 * <объём трубы магистрали> / dt)
    void setPipeVolume(double value);

    /// Задать давление со стороны магистрали
    void setPipePressure(double value);

    /// Задать поток из рукава
    void setHoseFlow(double value);

    /// Задать смещение точки крепления рукава от продольной оси (<0 - влево, >0 - вправо)
    void setShiftSide(double value);

    /// Задать смещение точки крепления рукава по продольной оси от оси автосцепки
    void setShiftCoord(double value);

    /// Получить коэффициент перетока через кран
    double getFlowCoeff() const;

    /// Получить поток в магистраль
    double getFlowToPipe() const;

    /// Получить давление в рукаве
    double getPressureToHose() const;

    /// Получить смещение точки крепления рукава от продольной оси (<0 - влево, >0 - вправо)
    double getShiftSide() const;

    /// Получить смещение точки крепления рукава по продольной оси от оси автосцепки
    double getShiftCoord() const;

protected:

    /// Заданное состояние крана: 0 - закрыт, 1 - открыт
    bool ref_state;

    /// Время переключения концевого крана, с
    double switch_time;

    /// Кран открыт
    bool is_opened;

    /// Давление в магистрали
    double p;

    /// Поток из рукава в магистраль
    double Q;

    /// Объём магистрали, к которой присоединён концевой кран
    double pipe_volume;

    /// Максимальное значение коэффициента перетока из рукава в магистраль
    /// (для устойчивого расчёта не более 0.15 * <объём трубы магистрали> / dt)
    double k_max_by_pipe_volume;

    /// Коэффициент перетока из рукава в магистраль при открытом кране
    double k_pipe;

    /// Коэффициент перетока из рукава в атмосферу при закрытом кране
    double k_atm;

    /// Смещение точки крепления рукава от продольной оси (<0 - влево, >0 - вправо)
    double shift_side;

    /// Смещение точки крепления рукава по продольной оси от оси автосцепки
    double shift_coord;

    virtual void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    virtual void stepDiscrete(double t, double dt);

    virtual void preStep(state_vector_t &Y, double t);

    virtual void load_config(CfgReader &cfg);
};

#endif // PNEUMO_ANGLECOCK_H
