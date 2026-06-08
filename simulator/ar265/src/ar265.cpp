#include    "ar265.h"

#include    "core/get_module.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
AutoMode265::AutoMode265() : BrakeAutoMode ()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AutoMode265::preStep(state_vector_t &Y, double t)
{
    (void) t;

    // Y[0] - давление в рабочей камере, заполняемой от воздухораспределителя
    // Y[1] - положение демпферного поршня

    double Qto_bc;
    double Qfrom_bc;

    if (Y[AUTO_MODE_WORK_PRESSURE] > p_lock)
    {
        // Положение демпферного поршня относительно пределов чувствительности
        double poz = std::clamp((Y[DEMPFER_LEVEL] - payload_min) / (payload_max - payload_min), 0.0, 1.0);
        // Коэффициент уменьшения давления в ТЦ
        double red_coeff = reduction_min + (reduction_max - reduction_min) * poz;
        // Требуемое давление в ТЦ
        double bc_ref = p_lock + red_coeff * (Y[AUTO_MODE_WORK_PRESSURE] - p_lock);

        // Разница давления в ТЦ и требуемого давления в ТЦ
        double bc_diff = A * (bc_ref - pBC);
        // Расход воздуха из камеры в ТЦ
        Qto_bc = std::clamp(bc_diff - 0.001, 0.0, coeffAirFlow) * (Y[AUTO_MODE_WORK_PRESSURE] - pBC);
        // Расход воздуха из ТЦ в атмосферу
        Qfrom_bc = std::clamp(-bc_diff, 0.0, coeffAirFlow) * pBC;
    }
    else
    {
        // Расход воздуха из камеры в ТЦ
        Qto_bc = coeffAirFlow * (Y[AUTO_MODE_WORK_PRESSURE] - pBC);
        // Расход воздуха из ТЦ в атмосферу
        Qfrom_bc = 0.0;
    }

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
    (void) t;

    dYdt[AUTO_MODE_WORK_PRESSURE] = Qc / Vc;

    if (Y[AUTO_MODE_WORK_PRESSURE] > p_lock)
    {
        dYdt[DEMPFER_LEVEL] = 0.0;
    }
    else
    {
        const double delta = payload_coeff - Y[DEMPFER_LEVEL];
        if (abs(delta) > 0.05)
        {
            dYdt[DEMPFER_LEVEL] = sign(delta) / motion_time;
        }
        else
        {
            dYdt[DEMPFER_LEVEL] = 20.0 * delta / motion_time;
        }
    }
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

GET_MODULE(AutoMode265)
