#ifndef     POWER_SUPPLY_H
#define     POWER_SUPPLY_H

#include    "device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class PowerSupply : public Device
{
public:

    PowerSupply(QObject *parent = Q_NULLPTR);

    ~PowerSupply() override;

    void setInputVoltage(double U_in);

    void setBatChargeCurrent(double I_charge) { this->I_charge = I_charge; }

    void setBatVoltage(double U_bat) { this->U_bat = U_bat; }

    double getPowerControlVoltage() const { return Ucc; }

    double getChargeVoltage() const { return U_charge; }

private:

    /// Входное напряжение переменного тока
    double U_in;

    /// Выходное напряжение подзаряда аккумуляторной батареи
    double U_charge;

    /// Напряжение аккумуляторной батареи
    double U_bat;

    /// Выходное напряжение питания цепей управления
    double Ucc;

    /// Ток заряда аккумуляторной батареи
    double I_charge;

    /// Ток нагрузки от цепей управления
    double In;

    /// Коэффициент передачи питания ЦУ
    double K1;

    /// Номинальное напряжение питания цепей управления
    double Ucc_nom;

    /// Уставка тока заряда АКБ
    double I_cy;

    /// Минимальное напряжение заряда АКБ
    double Uc_min;

    /// Максимальное напряжение заряда АКБ
    double Uc_max;

    /// Коэффициент передачи регулятора напряжения
    double K2;

    void ode_system(const state_vector_t &Y,
                    state_vector_t &dYdt,
                    double t) override;

    void stepDiscrete(double t, double dt) override;
};

#endif // POWER_SUPPLY_H
