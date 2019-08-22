//------------------------------------------------------------------------------
//
//      Магистральный пассажирский электровоз постоянного тока ЧС2т.
//      Дополнение для Russian Railway Simulator (RRS)
//
//      (c) RRS development team:
//          Дмитрий Притыкин (maisvendoo),
//          Николай Авилкин (avilkin.nick)
//
//      Дата: 21/08/2019
//
//------------------------------------------------------------------------------
#ifndef BV_H
#define BV_H

#include    "device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class BV : public Device
{
public:

    /// Конструктор
    BV(QObject *parent = Q_NULLPTR);

    /// Деструктор
    ~BV();

    void setP0(double P0)       { this->P0 = P0; }
    void setP1(double P1)       { this->P1 = P1; }

    void setUkr(double Ukr)     { this->Ukr = Ukr; }

    void setBVState(bool state) { this->BVstate = state; }
    void setVZState(bool state) { this->VZstate = state; }

    void setPhc(bool phc)       { this->phc = phc; }


    double getX()    { return getY(0); }
    double getUout() { return Uout; }


private:
    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);
    void load_config(CfgReader &cfg);
    void preStep(state_vector_t &Y, double t);
    void stepKeysControl(double t, double dt);


    CfgReader cfg_;

    double  Uout;
    double  Ukr;

    double Vn;
    double Vdk;

    double Fk;
    double Fp;

    double P0;
    double P1;

    double K1;
    double K2;

    double sdk;
    double Pdk;

    /// Состояние главного выключателя
    bool BVstate;

    /// Состояние возврата защиты
    bool VZstate;

    /// Питание удерживающей катушки
    bool phc;

};

#endif // BV_H
