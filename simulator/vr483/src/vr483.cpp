#include	"vr483.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
AirDist483::AirDist483() : AirDistributor ()
  , switchProfile(1)
  , switchPayload(1)
  , A(1.0)
{
    v[RK] = 0.006;
    v[ZK] = 0.0045;
    v[KDR] = 0.0005;
    Q.fill(0.0);
    k.fill(0.0);
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
        QString coeff = QString("d%1").arg(i);
        cfg.getDouble(secName, coeff, k[i]);
    }
    for (size_t i = 0; i < p.size(); ++i)
    {
        QString coeff = QString("p%1").arg(i);
        cfg.getDouble(secName, coeff, p[i]);
    }
    cfg.getDouble(secName, "A", A);
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
    double Qbp_as = k[0] * pf(pBP - pSR);
    // Расход воздуха между МК и ЗК через плунжер
    double Qmk_zk_pl = k[1] * hs_p(poz_d - p[2]) * (pBP - Y[ZK]);
    // Расход воздуха между МК и ЗК через клапан мягкости
    double Qmk_zk_km = k[2] * hs_p(Y[ZK] - Y[KDR] - p[0]) * (pBP - Y[ZK]);
    // Расход воздуха между ЗК и РК через корпус главного поршня
    double Qzk_rk_gp = k[3] * hs_n(poz_gp - p[9]) * (Y[ZK] - Y[RK]);
    // Расход воздуха между ЗК и РК через плунжер и диафрагму режима профиля пути
    double Qzk_rk_dp = k[4] * hs_p(poz_d - p[3]) * hs_p(Y[RK] - p[1]) * static_cast<double>(switchProfile) * (Y[ZK] - Y[RK]);

    // Торможение
    // Расход воздуха из ЗК в КДР при разрядке через плунжер
    double Qzk_kdr_pl = k[1] * hs_n(poz_d - p[4]) * (Y[ZK] - Y[KDR]);
    // Расход воздуха из МК в КДР при дополнительной разрядке
    double Qmk_kdr_dop = k[5] * hs_n(poz_d - p[5]) * hs_p(pBP - Y[KDR] - p[8]) * (pBP - Y[KDR]);
    // Расход воздуха из ЗК в КДР при дополнительной разрядке
    double Qzk_kdr_dop = k[6] * hs_n(poz_d - p[7]) * (Y[ZK] - Y[KDR]);
    // Расход воздуха из КДР в ТЦ
    double Qkdr_bc = k[7] * hs_n(poz_gp - p[10]) * (Y[KDR] - pBC);
    // Расход воздуха из КДР в атмосферу через осевой канал уравнительного поршня
    double Qkdr_atm = k[8] * hs_n(poz_gp - p[10]) * Y[KDR];
    // Расход воздуха из КДР в атмосферу дополнительно через атмосферный клапан
    double Qkdr_atm_dop = k[9] * hs_n(poz_d - p[6]) * Y[KDR];

    // Уравнительный поршень
    // Относительное положение уравнительного поршня (равновесное главному поршню)
    double poz_up = cut((poz_gp - p[11]) / (p[13] - p[11]), 0.0, 1.0);
    // Эквивалентное давление на уравнительный поршень от усилия основной пружины
    double pUP = p[14] + (p[15] - p[14]) * poz_up;
    // Эквивалентное давление на уравнительный поршень от усилия пружины гружёного режима
    double pUP_g = pf(p[16] + (p[17] - p[16]) * (poz_up - static_cast<double>(switchPayload) / 2.0) );

    // Взаимодействие с ТЦ
    // Разница давления в ТЦ и усилий от пружин уравнительного поршня
    double d_pBC = A * (pUP + pUP_g - pBC);
    // Расход воздуха из ЗР в ТЦ при быстром наполнении
    double Qas_bc_fast = cut(d_pBC, 0.0, k[10]) * hs_p(poz_gp - p[11]) * hs_n(poz_gp - p[12]) * (pSR - pBC);
    // Расход воздуха из ЗР в ТЦ при медленном наполнении
    double Qas_bc_slow = cut(d_pBC, 0.0, k[11]) * hs_p(poz_gp - p[12]) * (pSR - pBC);
    // Расход воздуха из ТЦ в атмосферу
    double Qbc_atm = max( cut(-d_pBC, 0.0, k[8]), hs_n(poz_gp - p[11]) ) * pBC;

    // Расход воздуха в РК
    Q[RK] = Qzk_rk_gp + Qzk_rk_dp;
    // Расход воздуха в ЗК
    Q[ZK] = Qmk_zk_pl + Qmk_zk_km - Qzk_rk_gp - Qzk_rk_dp - Qzk_kdr_pl - Qzk_kdr_dop;
    // Расход воздуха в КДР
    Q[KDR] = Qzk_kdr_pl + Qmk_kdr_dop + Qzk_kdr_dop - Qkdr_atm - Qkdr_atm_dop - Qkdr_bc;
    // Расход воздуха в ТЦ
    QBC = Qkdr_bc + Qas_bc_fast + Qas_bc_slow - Qbc_atm;
    // Расход воздуха в ЗР
    QSR = Qbp_as - Qas_bc_fast - Qas_bc_slow;
    // Расход воздуха из ТМ
    QBP = - Qbp_as - Qmk_zk_pl - Qmk_zk_km - Qmk_kdr_dop;

    DebugMsg = QString("483:RK%1|ZK%2|KDR%3|")
            .arg(10.0 * Y[RK], 6, 'f', 3)
            .arg(10.0 * Y[ZK], 6, 'f', 3)
            .arg(10.0 * Y[KDR], 6, 'f', 3);
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
