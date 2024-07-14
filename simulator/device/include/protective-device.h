#ifndef     MAIN_SWITCH_H
#define     MAIN_SWITCH_H

#include    "device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DEVICE_EXPORT ProtectiveDevice : public Device
{
public:

    ProtectiveDevice(QObject *parent = Q_NULLPTR);

    ~ProtectiveDevice();

    void setU_in(double value);

    double getU_out() const;

    void setState(bool is_on);

    void setReturn(bool is_return);

    void setHoldingCoilState(bool holding_coil);

    double getKnifePos() const;

    float getLampState() const;

    bool getState() const;

    /// Состояние звука включения
    sound_state_t getSoundOn() const;

    /// Состояние звука отключения
    sound_state_t getSoundOff() const;

private:

    /// Напряжение на входе ГВ
    double  U_in;

    /// Напряжение на выходе ГВ
    double  U_out;

    /// Сигнал включения ГВ
    bool    is_on;

    /// Возврат защиты
    bool    is_return;

    /// Состояние цепи питания удерживающей катушки
    bool    holding_coil;

    /// Условная скорость перемещения ножа разъединителя
    double  Vn;

    /// Условное усилие толкателя клапана отключения
    double  Fp;

    /// Условное усилие удерживающей катушки
    double  Fk;

    /// Условный объем дугогасительной камеры (ДК)
    double  Vdk;

    /// Коэффициент расхода при заполнении ДК
    double  K1;

    /// Коэффициент расхода при опорожнении ДК
    double  K2;

    /// Давление в резервуаре управления
    double p0;

    /// Давление срабатывания разрывных контактов
    double p1;

    float lamp_state;

    /// Состояние главного выключателя
    Trigger state;

    void preStep(state_vector_t &Y, double t);

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);    
};

#endif // MAIN_SWITCH_H
