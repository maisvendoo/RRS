#ifndef     MSUD_H
#define     MSUD_H

#include    "device.h"

#include    "msud-data.h"
#include    "vip-5600-defines.h"
#include    "shunts-module-defines.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class MSUD : public Device
{
public:

    MSUD(QObject *parent = Q_NULLPTR);

    ~MSUD() override;

    /// Общая инициализация МСУД
    void init();

    void setPowerVoltage(double Uc);

    void setInputData(const msud_input_t &msud_input)
    {
        this->msud_input = msud_input;
    }

    msud_output_t getOutputData() const
    {
        return this->msud_output;
    }

private:

    // Напряжение питания
    double  Uc;    

    // Минимально допустимое напряжение питания МСУД
    double  Uc_min;

    // Условное время загрузки МСУД после включения питания
    double  load_time;

    // Таймер загрузки МСУД
    Timer   *load_timer;

    bool is_fans_low_freq;

    Timer *normalFreqTimer;

    Timer *lowFreqTimer;

    Timer *fansBustTimer;

    Timer *runOutTimer;

    size_t fans_count;

    double fan_switch_timeout;

    double fan_bust_interval;

    double fan_runout_time;

    double I_fan_sw_min;

    double I_fan_sw_max;

    bool is_pchf_ignored;

    bool key_state_plus;

    bool old_key_state_plus;

    bool key_state_minus;

    bool old_key_state_minus;

    /// Коэффициент пропорциональной части регулятора тока в тяге
    double Ktp;

    /// Коэффициент интегральной части регулятора тока в тяге
    double Kti;

    /// Коэффициент пропорциональной части регулятора скорости в тяге
    double Ktv;

    /// Ошибка по току якоря ТЭД
    double dIa;

    /// Конструкционная скорость электровоза
    double Vmax;

    /// Ошибка по скорости
    double dV;

    /// Коэффициент интегральной части регулятора скорости
    double Ktvi;

    /// Коэффициент пропорциональной части регулятора тока в рекуперации
    double Krp;

    /// Коэффициент интегральной части регулятора тока в рекуперации
    double Kri;

    /// Коэффициент пропорциональной части регулятора скорости в рекуперации
    double Krv;

    /// Коэффициент интегральной части регулятора скорости в рекуперации
    double Krvi;

    /// Ограничение по току балластных резисторов
    double Ib_max;

    /// Ограничение по току обмотки возбуждения
    double If_max;

    /// Скорость перехода на постоянную величину тормозного усилия, км/ч
    double Vp;

    msud_input_t msud_input;

    msud_output_t msud_output;

    std::vector<zone_t> vip_zone;

     void ode_system(const state_vector_t &Y,
                     state_vector_t &dYdt,
                     double t) override;

     void preStep(state_vector_t &Y, double t) override;

     void stepDiscrete(double t, double dt) override;

     void load_config(CfgReader &cfg) override;

     void stepKeysControl(double t, double dt) override;

     // Обработка переходов мсуд из состояния в состояние
     void stateProcess(double t, double dt);

     // Общий сброс МСУД
     void reset();

     // Основной цикл работы МСУД
     void main_loop(double t, double dt);

     // Управление частотой вращения мотор-вентиляторов
     void motor_fans_control(double t, double dt);

     // Контроль давления в ТЦ
     void brake_cylinders_pressure_control(double t, double dt);

     // Управление тягой
     void traction_control(double t, double dt);

     // Ручное управление тягой
     void manual_traction_control(double t, double dt);

     // Авторегулирование в тяге
     void auto_traction_control(double t, double dt);

     // Сброс управления ВИП в тяге
     void reset_traction_control();

     // Выбор зоны ВИП в тяге по требуемому напряжению на ТЭД
     size_t select_traction_VIP_Zone(double Ud);

     // Расчет параметров работы ВИП, для обеспечения заданного напряжения
     void vip_control(double Ud);

     // Управление ослаблением возбуждения ТЭД в тяге
     void field_weak_control(double t, double dt);

     // Управление рекуперацией
     void recuperation_control(double t, double dt);

     // Авторегулирование в рекуперации
     void auto_recuperation_control(double t, double dt);

     // Ручное управление рекуперацией
     void manual_recuperation_control(double t, double dt);

     // Сброс управления ВИПи ВУВ в рекуперации
     void reset_recuperaion_control();

     // Регулирование тормозного тока
     void brake_current_regulator(double Ia_ref);

     // Отработка усиления торможения при ЭТ
     void emergancy_brake_boost();

private slots:

     void slotLoadTimer();

     void slotNormalFreqTimer();

     void slotLowFreqTimer();

     void slotFansBustTimer();

     void slotRunOutTimer();
};

#endif // MSUD_H
