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
class JointPneumoHoseEPB : public Joint
{
public:

    /// Конструктор
    JointPneumoHoseEPB();

    /// Деструктор
    ~JointPneumoHoseEPB();

    /// Шаг симуляции
    virtual void step(double t, double dt);

private:

    /// Состояние соединения рукавов
    bool is_connected;

    /// Количество соединений линий ЭПТ
    size_t epb_connected_num;

    /// Загрузка параметров из конфига
    virtual void load_config(CfgReader &cfg);
};

#endif // JOINT_PNEUMO_HOSE_H
