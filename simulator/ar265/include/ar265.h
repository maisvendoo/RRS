#ifndef     AR265
#define     AR265

#include    "brake-auto-mode.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class AutoMode265 : public BrakeAutoMode
{
public:

    AutoMode265();

    ~AutoMode265() = default;

private:

    enum
    {
        DEMPFER_LEVEL = 1, ///< Y[1] - Положение демпферного поршня
    };

    /// Расход воздуха в камеру
    double Qc = 0.0;
    /// Объём внутренней камеры
    double Vc = 0.001;
    /// Коэффициент для потока воздуха
    double coeffAirFlow = 0.05;

    /// Время перемещения демпферного поршня на полный рабочий ход, с
    double motion_time = 60.0;
    /// Давление, блокирующее перемещение демпферного поршня
    double p_lock = 0.03;

    /// Минимальное значение коэффициента загруженности
    double payload_min = 0.0;
    /// Максимальное значение коэффициента загруженности
    double payload_max = 0.8;
    /// Коэффициент уменьшения давления в ТЦ при минимальной загруженности
    double reduction_min = 0.4;
    /// Коэффициент уменьшения давления в ТЦ при максимальной загруженности
    double reduction_max = 1.0;

    /// Коэффициент чувствительности к разнице текущего и требуемого давления в ТЦ
    double A = 1.0;

    void load_config(CfgReader &cfg);

    void preStep(state_vector_t &Y, double t);

    virtual void ode_system(const state_vector_t &Y,
                            state_vector_t &dYdt,
                            double t);
};

#endif // AR265
