#ifndef     AR265
#define     AR265

#include    "brake-auto-mode.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
enum
{
    DEMPFER_LEVEL = 1, ///< Положение демпферного поршня
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class AutoMode265 : public BrakeAutoMode
{
public:

    AutoMode265();

    ~AutoMode265();

    void init(double pBP, double pFL);

private:

    /// Расход воздуха в камеру
    double Qc;
    /// Объём внутренней камеры
    double Vc;
    /// Коэффициент для потока воздуха
    double coeffAirFlow;

    /// Время перемещения демпферного поршня на полный рабочий ход, с
    double motion_time;
    /// Давление, блокирующее перемещение демпферного поршня
    double p_lock;

    /// Минимальное значение коэффициента загруженности
    double payload_min;
    /// Максимальное значение коэффициента загруженности
    double payload_max;
    /// Коэффициент уменьшения давления в ТЦ при минимальной загруженности
    double reduction_min;
    /// Коэффициент уменьшения давления в ТЦ при максимальной загруженности
    double reduction_max;

    /// Коэффициент чувствительности к разнице текущего и требуемого давления в ТЦ
    double A;

    void load_config(CfgReader &cfg);

    void preStep(state_vector_t &Y, double t);

    virtual void ode_system(const state_vector_t &Y,
                            state_vector_t &dYdt,
                            double t);
};

#endif // AR265
