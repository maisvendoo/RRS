#include	"vr483.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
AirDist483::AirDist483() : AirDistributor ()
  , switchProfile(1)
  , switchPayload(1)
{
    v[RK] = 0.006;
    v[ZK] = 0.0045;
    v[KDR] = 0.0005;
    Q.fill(0.0);
    k.fill(0.0);
    A.fill(0.0);
    p.fill(0.0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
AirDist483::~AirDist483()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AirDist483::init(double pBP, double pFL)
{
    Q_UNUSED(pFL)

    y[RK] = pBP;
    y[ZK] = pBP;
    y[KDR] = 0.0;
}

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
    for (size_t i = 0; i < v.size(); ++i)
    {
        QString coeff = QString("v%1").arg(i);
        cfg.getDouble(secName, coeff, v[i]);
    }

    // Коэффициенты
    for (size_t i = 0; i < k.size(); ++i)
    {
        QString coeff = QString("k%1").arg(i);
        cfg.getDouble(secName, coeff, k[i]);
    }
    for (size_t i = 0; i < p.size(); ++i)
    {
        QString coeff = QString("p%1").arg(i);
        cfg.getDouble(secName, coeff, p[i]);
    }
    for (size_t i = 0; i < p.size(); ++i)
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
    Q_UNUSED(t)

    // Y[0] - Давление в рабочей камере (РК)
    // Y[1] - Давление в золотниковой камере (ЗК)
    // Y[2] - Давление в каналах дополнительной разрядки (КДР)

    // Условное положение диафрагмы - разница давлений в МК и ЗК
    double poz_d = pBP - Y[ZK];
    // Условное положение главного поршня - разница давлений в РК и ЗК
    double poz_gp = Y[RK] - Y[ZK];

    // Расчёт перетоков воздуха между разными элементами воздухораспределителя

    // Зарядка/отпуск, взаимодействие камер
    // Расход воздуха из ТМ в ЗР через обратный клапан
    double Q_bp_sr = k[0] * cut((pBP - pSR), 0.0, A[0]);
    // Расход воздуха между МК и ЗК через плунжер
    double Q_mk_zk_pl = k[1] * hs_p(poz_d - p[2]) * min((pBP - Y[ZK]), A[1]);
    // Расход воздуха между МК и ЗК через клапан мягкости
    double Q_mk_zk_km = k[2] * hs_p(Y[ZK] - Y[KDR] - p[0]) * cut((pBP - Y[ZK]), -A[2], A[2]);
    // Расход воздуха между ЗК и РК через корпус главного поршня
    double Q_zk_rk_gp = k[3] * hs_n(poz_gp - p[10]) * (Y[ZK] - Y[RK]);
    // Коэффициент через отжатую диафрагму режима профиля пути в равнинном режиме
    double profile_d_coeff = k[4] * static_cast<double>(switchProfile) * hs_p(Y[RK] - p[1]);
    // Расход воздуха между ЗК и РК через плунжер и диафрагму режима профиля пути
    double Q_zk_rk_pd = profile_d_coeff * hs_p(poz_d - p[3]) * (Y[ZK] - Y[RK]);
    // Расход воздуха между МК и РК через плунжер и диафрагму режима профиля пути
    double Q_mk_rk_pd = profile_d_coeff * hs_p(poz_d - p[2]) * (pBP - Y[RK]);

    // Торможение
    // Расход воздуха из ЗК в КДР при мягкой разрядке через плунжер
    // Условно считаем поток через промежуточную камеру
    // с клапаном доп. разрядки МК, с давлением, близким к МК(ТМ)
    double Q_zk_kdr_pl = k[1] * cut(A[3] * (p[4] - poz_d), 0.0, 1.0) * (Y[ZK] - pBP);
    // Расход воздуха из МК в КДР при дополнительной разрядке
    double Q_mk_kdr_dop = k[5] * hs_p(p[5] - poz_d) * cut(A[4] * (pBP - Y[KDR] - p[8]), 0.0, 1.0) * (pBP - Y[KDR]);
    // Расход воздуха из ЗК в КДР при дополнительной разрядке
    // TODO // настроить доп.разрядку ЗК более быстрой и непрерывной
    // TODO // до возвращения диафрагмы в перекрышу доп.разрядки ТМ,
    // TODO // т.е. реализовать остановку дутья при несрабатывании на торможение
    // TODO // не критично, поскольку несрабатывание не реализовано
    double Q_zk_kdr_dop = k[6] * cut(A[5] * (p[7] - poz_d), 0.0, 1.0) * (Y[ZK] - Y[KDR]);
    // Расход воздуха из КДР в ТЦ
    double Q_kdr_bc = k[7] * hs_p(p[11] - poz_gp) * (Y[KDR] - pBC);
    // Расход воздуха из КДР в атмосферу через осевой канал уравнительного поршня
    double Q_kdr_atm = k[8] * hs_p(p[11] - poz_gp) * Y[KDR];
    // Расход воздуха из КДР в атмосферу дополнительно через атмосферный клапан
    double Q_kdr_atm_dop = k[9] * cut(A[6] * (pf(p[6] - poz_d) + pf(Y[KDR] - p[9])), 0.0, 1.0) * Y[KDR];

    // Уравнительный поршень
    // Относительное положение уравнительного поршня (равновесное главному поршню)
    double poz_up = cut((poz_gp - p[11]) / (p[13] - p[11]), 0.0, 1.0);
    // Эквивалентное давление на уравнительный поршень от усилия основной пружины
    double pUP = p[14] + (p[15] - p[14]) * poz_up;
    // Эквивалентное давление на уравнительный поршень от усилия пружины гружёного режима
    double pUP_g = pf( p[16] + (p[17] - p[16]) * (poz_up - (1 - static_cast<double>(switchPayload) / 2.0)) );

    // Взаимодействие с ТЦ
    // Разница давления в ТЦ и усилий от пружин уравнительного поршня
    double d_pBC = pUP + pUP_g - pBC;
    // Расход воздуха из ЗР в ТЦ при быстром наполнении
    double Q_sr_bc_fast = k[10] * cut(A[7] * d_pBC, 0.0, 1.0) * hs_p(poz_gp - p[11]) * hs_n(poz_gp - p[12]) * (pSR - pBC);
    // Расход воздуха из ЗР в ТЦ при медленном наполнении
    double Q_sr_bc_slow = k[11] * cut(A[7] * d_pBC, 0.0, 1.0) * hs_p(poz_gp - p[12]) * (pSR - pBC);
    // Расход воздуха из ТЦ в атмосферу
    double Q_bc_atm = k[8] * max( cut(-A[8] * d_pBC, 0.0, 1.0), hs_n(poz_gp - p[11]) ) * pBC;

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
/*
    DebugMsg = QString("483:RK%1|ZK%2|KDR%3|poz_d%4|poz_gp%5|poz_up:%6")
            .arg(10.0 * Y[RK], 6, 'f', 3)
            .arg(10.0 * Y[ZK], 6, 'f', 3)
            .arg(10.0 * Y[KDR], 6, 'f', 3)
            .arg(poz_d, 6, 'f', 3)
            .arg(poz_gp, 6, 'f', 3)
            .arg(poz_up, 6, 'f', 3);
*/
/*
//    QString("  time  ; pBP   ; pBC   ; pSR   ; pRK   ; pZK   ; pKDR  ; pBCref; BPsr   ; MKzk km; MKzk pl; ZKrk gp; ZKrk pd; MKrk pd; ZKkdr  ; ZKkdr d; MKkdr d; KDRbc  ; KDRatm ; KDRatmd; SRbc f ; SRbc s ; BCatm  ; poz d ; poz gp; poz up");
    DebugMsg = QString("%1;%2;%3;%4;%5;%6;%7;%8;%9;%10;%11;%12;%13;%14;%15;%16;%17;%18;%19;%20;%21;%22;%23;%24;%25;26")
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
*/
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AirDist483::ode_system(const state_vector_t &Y,
                            state_vector_t &dYdt,
                            double t)
{
    Q_UNUSED(t)
    Q_UNUSED(Y)

    // Изменение давления в РК
    dYdt[RK] = Q[RK] / v[RK];
    // Изменение давления в ЗК
    dYdt[ZK] = Q[ZK] / v[ZK];
    // Изменение давления в КДР
    dYdt[KDR] = Q[KDR] / v[KDR];
}

GET_AIR_DISTRIBUTOR(AirDist483)
