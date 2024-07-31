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
    PneumoAngleCock(int key_code = 0, QObject *parent = Q_NULLPTR);

    /// Деструктор
    virtual ~PneumoAngleCock();

    /// Задать управляющую клавишу
    void setKeyCode(int key_code);

    /// Закрыть концевой кран
    void close();

    /// Открыть концевой кран
    void open();

    /// Cостояние концевого крана
    bool isOpened() const;

    /// Положение рукоятки: от 0.0 (кран закрыт) до 1.0 (кран открыт)
    double getHandlePosition() const;

    /// Задать объём трубы магистрали для вычисления максимального коэффициента перетока
    /// (для устойчивого расчёта не более 0.5 * <объём трубы магистрали> / dt)
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

    enum {
        NUM_SOUNDS = 4,
        CHANGE_SOUND = 0,   ///< Звук переключения концевого крана
        OPEN_SOUND = 1,     ///< Звук открытия концевого крана
        CLOSE_SOUND = 2,    ///< Звук перекрытия концевого крана
        PIPE_DRAIN_FLOW_SOUND = 3 ///< Звук опорожнения рукава через атмосферное отверстие
    };
    /// Состояние звука
    virtual sound_state_t getSoundState(size_t idx = CHANGE_SOUND) const;

    /// Сигнал состояния звука
    virtual float getSoundSignal(size_t idx = CHANGE_SOUND) const;

protected:

    /// Код управляющей клавиши
    int keyCode = 0;

    /// Время переключения концевого крана, с
    double switch_time = 0.2;

    /// Кран открыт
    bool is_opened = false;

    /// Давление в магистрали
    double p = 0.0;

    /// Поток из рукава в магистраль
    double Q = 0.0;

    /// Объём магистрали, к которой присоединён концевой кран
    double pipe_volume = 1.0e8;

    /// Максимальное значение коэффициента перетока из рукава в магистраль
    /// (для устойчивого расчёта не более 0.5 * <объём трубы магистрали> / dt)
    double k_max_by_pipe_volume = 0.8;

    /// Коэффициент перетока из рукава в магистраль при открытом кране
    double k_pipe = 0.8;

    /// Коэффициент перетока из рукава в атмосферу при закрытом кране
    double k_atm = 0.1;

    /// Смещение точки крепления рукава от продольной оси (<0 - влево, >0 - вправо)
    double shift_side = 0.0;

    /// Смещение точки крепления рукава по продольной оси от оси автосцепки
    double shift_coord = 0.0;

    /// Коэффициент громкости озвучки к потоку опорожнения рукава через атмосферное отверстие
    double K_sound = 2.5;

    /// Заданное состояние крана: 0 - закрыт, 1 - открыт
    Trigger ref_state;

    /// Звук потока опорожнения рукава через атмосферное отверстие
    sound_state_t atm_flow_sound = sound_state_t();

    virtual void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    virtual void preStep(state_vector_t &Y, double t);

    virtual void stepKeysControl(double t, double dt);

    /// Загрузка параметров из конфигурационного файла
    virtual void load_config(CfgReader &cfg);
};

#endif // PNEUMO_ANGLECOCK_H
