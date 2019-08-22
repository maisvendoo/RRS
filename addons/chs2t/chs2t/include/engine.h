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
#ifndef ENGINE_H
#define ENGINE_H

#include "device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class Engine : public Device
{
public:

    ///Конструктор
    Engine(QObject *parent = Q_NULLPTR);

    ///Деструктор
    ~Engine();

    void setPoz(int poz) { this->poz = poz; }

    void setR(double R) { this->R = R; }

private:
    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);
    void load_config(CfgReader &cfg);
    void preStep(state_vector_t &Y, double t);
    void stepKeysControl(double t, double dt);

    int poz;

    int n;

    double R;

};

#endif // ENGINE_H
