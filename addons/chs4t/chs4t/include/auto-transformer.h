//------------------------------------------------------------------------------
//
//      Магистральный пассажирский электровоз переменного тока ЧС4т.
//      Дополнение для Russian Railway Simulator (RRS)
//
//      (c) RRS development team:
//          Дмитрий Притыкин (maisvendoo),
//          Николай Авилкин (avilkin.nick)
//
//      Дата: 16/06/2019
//
//------------------------------------------------------------------------------
#ifndef AUTOTRANSFORMER_H
#define AUTOTRANSFORMER_H

#include "device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class AutoTransformer : public Device
{
public:

    /// Конструктор
    AutoTransformer(QObject *parent = Q_NULLPTR);

    /// Деструктор
    ~AutoTransformer();

    void setUin(double Uin) { this->Uin = Uin; }
    double getUout()        { return Uout; }

private:
    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);
    void load_config(CfgReader &cfg);
    void preStep(state_vector_t &Y, double t);
    void stepKeysControl(double t, double dt);

    double Uin;
    double Uout;
    int nPoz;

};

#endif // AUTOTRANSFORMER_H
