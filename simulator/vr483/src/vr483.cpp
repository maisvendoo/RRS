#include    "vr483.h"

#include    "math-funcs.h"
#include    "core/get_module.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
AirDist483::AirDist483() : AirDistributor ()
{
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AirDist483::init(double pBP, double pFL)
{
    (void) pFL;

    setY(RK, pBP);
    setY(ZK, pBP);
    setY(KDR, 0.0);
}

#ifndef NDEBUG
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString AirDist483::getDebugMsg() const
{
    return DebugMsg;
}
#endif

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AirDist483::load_config(CfgReader &cfg)
{
    QString secName = "Device";

    // Состояние переключателей режимов
    cfg.getInt(secName, "switchProfile", switchProfile);
    cfg.getInt(secName, "switchPayload", switchPayload);

    // Объёмы камер
    for (size_t i = 0; i < NUM_VOLUMES; ++i)
    {
        double tmp = 0.0;
        QString coeff = QString("v%1").arg(i);
        cfg.getDouble(secName, coeff, tmp);
        if (tmp > 0.0)
            v[i] = tmp;
    }

    // Коэффициенты
    for (size_t i = 0; i < NUM_COEFFS; ++i)
    {
        QString coeff = QString("k%1").arg(i);
        cfg.getDouble(secName, coeff, k[i]);
    }
    for (size_t i = 0; i < NUM_PRESSURES; ++i)
    {
        QString coeff = QString("p%1").arg(i);
        cfg.getDouble(secName, coeff, p[i]);
    }
    for (size_t i = 0; i < NUM_SENSIVITY_COEFFS; ++i)
    {
        QString coeff = QString("A%1").arg(i);
        cfg.getDouble(secName, coeff, A[i]);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AirDist483::preStep(state_vector_t &Y, double t)
{
    (void) t;

    // Условное положение диафрагмы - разница давлений в МК и ЗК
    const double poz_d = pBP - Y[ZK];
    // Условное положение главного поршня - разница давлений в РК и ЗК
    const double poz_gp = Y[RK] - Y[ZK];

    // Расчёт перетоков воздуха между разными элементами воздухораспределителя

    // Зарядка/отпуск, взаимодействие камер
    // Расход воздуха из ТМ в ЗР через обратный клапан
    const double Q_bp_sr =   std::clamp(A[A_SR_REFILL] * (pBP - pSR), 0.0, 1.0)
                           * k[K_SR_REFILL];
    // Расход воздуха между МК и ЗК через плунжер
    const double Q_mk_zk_pl =   hs_p(poz_d - p[P_MK_ZK_PLUNGER])
                              * std::clamp(A[A_PLUNGER] * (pBP - Y[ZK]), -1.0, 1.0)
                              * (k[K_PLUNGER] + k[K_MK_PLUNGER]);
    // Расход воздуха между МК и ЗК через клапан мягкости
    const double Q_mk_zk_km =   hs_p(Y[ZK] - Y[KDR] - p[P_SOFTNESS])
                              * std::clamp(A[A_SOFTNESS] * (pBP - Y[ZK]), -1.0, 1.0)
                              * k[K_SOFTNESS];
    // Расход воздуха между ЗК и РК через корпус главного поршня
    const double Q_zk_rk_gp =   hs_n(poz_gp - p[P_RK_ZK_SHUTOFF])
                              * (Y[ZK] - Y[RK])
                              * k[K_SIDE_HOLE];
    // Отжатие диафрагмы режима профиля пути в равнинном режиме
    const double is_flat = static_cast<double>(switchProfile) * hs_p(Y[RK] - p[P_RK_PROFILE]);
    // Расход воздуха между ЗК и РК через плунжер и диафрагму режима профиля пути
    const double Q_zk_rk_pd =   is_flat * hs_p(poz_d - p[P_ZK_RK_PLUNGER])
                              * std::clamp(A[A_PLUNGER] * (Y[ZK] - Y[RK]), -1.0, 1.0)
                              * k[K_RK_PROFILE];
    // Расход воздуха между МК и РК через плунжер и диафрагму режима профиля пути
    const double Q_mk_rk_pd =   is_flat * hs_p(poz_d - p[P_MK_ZK_PLUNGER])
                              * std::clamp(A[A_PLUNGER] * (pBP - Y[RK]), -1.0, 1.0)
                              * (k[K_RK_PROFILE] + k[K_MK_PLUNGER]);

    // Торможение
    // Расход воздуха из ЗК в КДР при мягкой разрядке через плунжер
    // Условно считаем поток в промежуточную камеру с клапаном доп. разрядки МК
    // с давлением, близким к давлению МК (к давлению ТМ)
    const double Q_zk_kdr_pl =   std::clamp(A[A_ZK_DISCHARGE] * (p[P_ZK_DISCHARGE] - poz_d), 0.0, 1.0)
                               * (Y[ZK] - pBP)
                               * k[K_PLUNGER];
    // Расход воздуха из МК в КДР при дополнительной разрядке
    const double Q_mk_kdr_dop =   hs_p(p[P_MK_DISCHARGE] - poz_d)
                                * std::clamp(A[A_MK_DISCHARGE] * (pBP - Y[KDR] - p[P_MK_KDR_EXTRA]), 0.0, 1.0)
                                * (pBP - Y[KDR])
                                * k[K_MK_DISCHARGE];
    // Расход воздуха из ЗК в КДР при дополнительной разрядке
    const double Q_zk_kdr_dop =   std::clamp(A[A_ZK_KDR_EXTRA] * (p[P_ZK_KDR_EXTRA] - poz_d), 0.0, 1.0)
                                * (Y[ZK] - Y[KDR])
                                * k[K_ZK_KDR_EXTRA];
    // Расход воздуха из КДР в ТЦ
    const double Q_kdr_bc =   hs_p(p[P_BC_FILL_BEGIN] - poz_gp)
                            * (Y[KDR] - pBC)
                            * k[K_BC_PREFILL];
    // Расход воздуха из КДР в атмосферу через осевой канал уравнительного поршня
    const double Q_kdr_atm =   hs_p(p[P_BC_FILL_BEGIN] - poz_gp)
                             * Y[KDR]
                             * k[K_BC_DISCHARGE];
    // Расход воздуха из КДР в атмосферу дополнительно через атмосферный клапан
    const double Q_kdr_atm_dop =   std::min(A[A_KDR_DISCHARGE] * (pf(p[P_KDR_DISCHARGE] - poz_d) + pf(Y[KDR] - p[P_KDR_EXTRA])), 1.0)
                                 * Y[KDR]
                                 * k[K_KDR_EXTRA];

    // Уравнительный поршень
    // Относительное положение уравнительного поршня (равновесное главному поршню)
    const double poz_up = std::clamp((poz_gp - p[P_BC_FILL_BEGIN]) / (p[P_BC_FILL_MAX] - p[P_BC_FILL_BEGIN]), 0.0, 1.0);
    // Относительное положение уравнительного поршня к переключателю гружёного режима
    const double poz_up_g = pf(poz_up - (1 - static_cast<double>(switchPayload) / 2.0));
    // Эквивалентное давление на уравнительный поршень от усилия основной пружины
    const double pUP =   p[P_SPRING_MAIN_BEGIN]
                       + poz_up * (p[P_SPRING_MAIN_MAX] - p[P_SPRING_MAIN_BEGIN]);
    // Эквивалентное давление на уравнительный поршень от усилия пружины гружёного режима
    const double pUP_g =   p[P_SPRING_LOAD_BEGIN]
                         + poz_up_g * (p[P_SPRING_LOAD_MAX] - p[P_SPRING_LOAD_BEGIN]);

    // Взаимодействие с ТЦ
    // Разница давления в ТЦ и усилий от пружин уравнительного поршня
    const double d_pBC = pUP + pUP_g - pBC;
    // Уравнительный поршень открывает тормозной клапан
    const double d_pBC_fill = std::clamp(A[A_DIFF_BC_FILL] * d_pBC, 0.0, 1.0);
    // Уравнительный поршень открывает атмосферный осевой канал
    const double d_pBC_empty = std::clamp(-A[A_DIFF_BC_EMPTY] * d_pBC, 0.0, 1.0);

    // Расход воздуха из ТЦ в атмосферу
    const double Q_bc_atm =   max( d_pBC_empty, hs_n(poz_gp - p[P_BC_FILL_BEGIN]) )
                            * pBC
                            * k[K_BC_DISCHARGE];
    // Расход воздуха из ЗР в ТЦ при быстром наполнении
    const double Q_sr_bc_fast =   d_pBC_fill * hs_p(poz_gp - p[P_BC_FILL_BEGIN]) * hs_n(poz_gp - p[P_BC_FILL_SLOW])
                                * (pSR - pBC)
                                * k[K_BC_FILL_FAST];
    // Расход воздуха из ЗР в ТЦ при медленном наполнении
    const double Q_sr_bc_slow =   d_pBC_fill * hs_p(poz_gp - p[P_BC_FILL_SLOW])
                                * (pSR - pBC)
                                * k[K_BC_FILL_SLOW];

    // Расход воздуха в РК
    Q[RK] = Q_zk_rk_gp + Q_zk_rk_pd + Q_mk_rk_pd;
    // Расход воздуха в ЗК
    Q[ZK] = Q_mk_zk_pl + Q_mk_zk_km - Q_zk_rk_gp - Q_zk_rk_pd - Q_zk_kdr_pl - Q_zk_kdr_dop;
    // Расход воздуха в КДР
    Q[KDR] = Q_zk_kdr_pl + Q_mk_kdr_dop + Q_zk_kdr_dop - Q_kdr_atm - Q_kdr_atm_dop - Q_kdr_bc;
    // Расход воздуха в ТЦ
    QBC = Q_kdr_bc + Q_sr_bc_fast + Q_sr_bc_slow - Q_bc_atm;
    // Расход воздуха в ЗР
    QSR = Q_bp_sr - Q_sr_bc_fast - Q_sr_bc_slow;
    // Расход воздуха в ТМ
    QBP = - Q_bp_sr - Q_mk_zk_pl - Q_mk_zk_km - Q_mk_rk_pd - Q_mk_kdr_dop;

#ifndef NDEBUG
//    QString("  time  ; pBP   ; pBC   ; pSR   ; pRK   ; pZK   ; pKDR  ; pBCref; BPsr   ; MKzk km; MKzk pl; ZKrk gp; ZKrk pd; MKrk pd; ZKkdr  ; ZKkdr d; MKkdr d; KDRbc  ; KDRatm ; KDRatmd; SRbc f ; SRbc s ; BCatm  ; poz d ; poz gp; poz up");
    DebugMsg = QString("%1;%2;%3;%4;%5;%6;%7;%8;%9;%10;%11;%12;%13;%14;%15;%16;%17;%18;%19;%20;%21;%22;%23;%24;%25;%26")
                   .arg(t, 8, 'f', 3)
                   .arg(10*pBP, 7, 'f', 5)
                   .arg(10*pBC, 7, 'f', 5)              //%3
                   .arg(10*pSR, 7, 'f', 5)
                   .arg(10*Y[RK], 7, 'f', 5)
                   .arg(10*Y[ZK], 7, 'f', 5)            //%6
                   .arg(10*Y[KDR], 7, 'f', 5)
                   .arg(10*(pUP+pUP_g), 7, 'f', 5)
                   .arg(10000*Q_bp_sr, 8, 'f', 5)       //%9
                   .arg(10000*Q_mk_zk_km, 8, 'f', 5)
                   .arg(10000*Q_mk_zk_pl, 8, 'f', 5)
                   .arg(10000*Q_zk_rk_gp, 8, 'f', 5)    //%12
                   .arg(10000*Q_zk_rk_pd, 8, 'f', 5)
                   .arg(10000*Q_mk_rk_pd, 8, 'f', 5)
                   .arg(10000*Q_zk_kdr_pl, 8, 'f', 5)   //%15
                   .arg(10000*Q_zk_kdr_dop, 8, 'f', 5)
                   .arg(10000*Q_mk_kdr_dop, 8, 'f', 5)
                   .arg(10000*Q_kdr_bc, 8, 'f', 5)      //%18
                   .arg(10000*Q_kdr_atm, 8, 'f', 5)
                   .arg(10000*Q_kdr_atm_dop, 8, 'f', 5)
                   .arg(10000*Q_sr_bc_fast, 8, 'f', 5)  //%21
                   .arg(10000*Q_sr_bc_slow, 8, 'f', 5)
                   .arg(10000*Q_bc_atm, 8, 'f', 5)
                   .arg(poz_d, 7, 'f', 4)            //%24
                   .arg(poz_gp, 7, 'f', 4)
                   .arg(poz_up, 7, 'f', 4);
#endif
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AirDist483::ode_system(const state_vector_t &Y,
                            state_vector_t &dYdt,
                            double t)
{
    (void) t;
    (void) Y;

    // Изменение давления в РК
    dYdt[RK] = Q[RK] / v[RK];
    // Изменение давления в ЗК
    dYdt[ZK] = Q[ZK] / v[ZK];
    // Изменение давления в КДР
    dYdt[KDR] = Q[KDR] / v[KDR];
}

GET_MODULE(AirDist483)
