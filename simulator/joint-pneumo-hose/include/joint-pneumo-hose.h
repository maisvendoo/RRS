#ifndef     JOINT_PNEUMO_HOSE_H
#define     JOINT_PNEUMO_HOSE_H

#include    "device-joint.h"

#include    "pneumo-hose-data.h"

enum
{
    NUM_CONNECTORS = 2,
    FWD = 0,
    BWD = 1
};

//------------------------------------------------------------------------------
// Соединение пневматических рукавов
//------------------------------------------------------------------------------
class JointPneumoHose : public Joint
{
public:

    /// Конструктор
    JointPneumoHose();

    /// Деструктор
    ~JointPneumoHose();

    /// Шаг симуляции
    virtual void step(double t, double dt);

private:

    /// Состояние соединения рукавов
    bool is_connected;

    /// Загрузка параметров из конфига
    virtual void load_config(CfgReader &cfg);
};

#endif // JOINT_PNEUMO_HOSE_H
