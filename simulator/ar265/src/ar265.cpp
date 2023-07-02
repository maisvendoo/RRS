#include	"ar265.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
AutoMode265::AutoMode265() : BrakeAutoMode ()
  , Qc(0.0)
  , Vc(0.001)
  , coeffAirFlow(0.05)
  , motion_time(60.0)
  , p_lock(0.05)
  , payload_min(0.0)
  , payload_max(0.8)
  , reduction_min(0.4)
  , reduction_max(1.0)
  , A(1.0)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
AutoMode265::~AutoMode265()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AutoMode265::init(double pBP, double pFL)
{
    Q_UNUSED(pBP)
    Q_UNUSED(pFL)
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AutoMode265::preStep(state_vector_t &Y, double t)
{
    Q_UNUSED(t)

    // Y[0] - давление в рабочей камере, заполняемой от воздухораспределителя
    // Y[1] - положение демпферного поршня

    // Положение демпферного поршня относительно пределов чувствительности
    double poz = cut((Y[DEMPFER_LEVEL] - payload_min) / (payload_max - payload_min), 0.0, 1.0);
    // Коэффициент уменьшения давления в ТЦ
    double red_coeff = reduction_min + (reduction_max - reduction_min) * poz;

    // Разница давления в ТЦ и требуемого давления в ТЦ
    double bc_diff = A * (red_coeff * Y[AUTO_MODE_WORK_PRESSURE] - pBC);
    // Расход воздуха из камеры в ТЦ
    double Qto_bc = cut(bc_diff, 0.0, coeffAirFlow) * pf(Y[AUTO_MODE_WORK_PRESSURE] - pBC);
    // Расход воздуха из ТЦ в атмосферу
    double Qfrom_bc = cut(-bc_diff, 0.0, coeffAirFlow) * pBC;

    // Итоговый расход воздуха в ТЦ
    QBC = Qto_bc - Qfrom_bc;
    // Итоговый расход воздуха в камеру
    Qc = QadBC - Qto_bc;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AutoMode265::ode_system(const state_vector_t &Y,
                            state_vector_t &dYdt,
                            double t)
{
    Q_UNUSED(Y)
    Q_UNUSED(t)

    dYdt[AUTO_MODE_WORK_PRESSURE] = Qc / Vc;
    dYdt[DEMPFER_LEVEL] = hs_p(p_lock - Y[AUTO_MODE_WORK_PRESSURE])
                        * (payload_coeff - Y[DEMPFER_LEVEL]) / motion_time;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AutoMode265::load_config(CfgReader &cfg)
{
    QString secName = "Device";

    cfg.getDouble(secName, "Vc", Vc);
    cfg.getDouble(secName, "coeffAirFlow", coeffAirFlow);
    cfg.getDouble(secName, "motion_time", motion_time);
    cfg.getDouble(secName, "p_lock", p_lock);

    cfg.getDouble(secName, "payload_min", payload_min);
    cfg.getDouble(secName, "payload_max", payload_max);
    cfg.getDouble(secName, "reduction_min", reduction_min);
    cfg.getDouble(secName, "reduction_max", reduction_max);

}

GET_BRAKE_AUTOMODE(AutoMode265)
