#ifndef DACO_H
#define DACO_H

#include "device.h"


//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class Dako : public Device
{
public:

    ///Конструктор
    Dako(QObject *parent = Q_NULLPTR);

    ///Деструктор
    ~Dako();

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    /// Загрузка данных из конфигурационных файлов
    void load_config(CfgReader &cfg);

    void preStep(state_vector_t &Y, double t);
    void stepKeysControl(double t, double dt);

};


#endif // DACO_H
