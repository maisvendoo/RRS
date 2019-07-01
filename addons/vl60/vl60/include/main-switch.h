#ifndef     MAIN_SWITCH_H
#define     MAIN_SWITCH_H

#include    "device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class MainSwitch : public Device
{
public:

    MainSwitch(QString config_path,  QObject *parent = Q_NULLPTR);

    ~MainSwitch();

    void setU_in(double value);

    double getU_out() const;

    void setState(bool is_on);

    void setReturn(bool is_return);

    void setHoldingCoilState(bool holding_coil);

    double getKnifePos() const;

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

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);

    void load_config(QString cfg_path);
};

#endif // MAIN_SWITCH_H
