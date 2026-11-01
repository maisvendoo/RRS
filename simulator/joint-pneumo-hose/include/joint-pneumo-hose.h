#ifndef     JOINT_PNEUMO_HOSE_H
#define     JOINT_PNEUMO_HOSE_H

#include    "device-joint.h"

#include    "pneumo-hose-data.h"
#include    "vec3.h"

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

    /// Точки крепления
    dvec3 coord_fwd = {0.0, 0.0, 0.0};
    dvec3 coord_bwd = {0.0, 0.0, 0.0};

    /// Точки, куда будут стремится гибкие части рукавов (до соединительной головки)
    dvec3 point_fwd = {0.0, 0.0, 0.0};
    dvec3 point_bwd = {0.0, 0.0, 0.0};

    /// Углы поворота соединительных головок вверх
    double fwd_angle = 0.0;
    double bwd_angle = 0.0;

    /// Состояние соединения рукавов
    bool ref_connected = false;
    bool is_connected = false;

    /// Расчёт соединения рукавов
    void stepConnectionCheck();

    /// Расчёт потоков через рукава
    void stepFlowCalc();

    /// Сбросить параметры соединения, если рукава разъединены
    void reset();

    /// Задать параметры соединения в рукава
    void apply();

    /// Загрузка параметров из конфига
    virtual void load_config(CfgReader &cfg);
};

#endif // JOINT_PNEUMO_HOSE_H
