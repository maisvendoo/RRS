#include	"vr292.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
AirDist292::AirDist292() : AirDistributor ()
  , long_train_mode(0)
  , Vkd(1.0e-3)
  , Qkd(0.0)
  , disjunction_z_pos(0.0)
  , main_z_pos(0.0)
  , main_z_eps(0.015)
{
    p.fill(0.0);
    K.fill(0.0);
    A.fill(0.0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
AirDist292::~AirDist292()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AirDist292::init(double pBP, double pFL)
{
    Q_UNUSED(pBP)
    Q_UNUSED(pFL)
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AirDist292::preStep(state_vector_t &Y, double t)
{
    Q_UNUSED(Y)
    Q_UNUSED(t)

    // Условное положение магистрального поршня и отсекательного золотника
    // с учётом трения
    disjunction_z_pos = cut(disjunction_z_pos,
                            pSR - pBP - (disjunction_z_eps / 2.0),
                            pSR - pBP + (disjunction_z_eps / 2.0));

    // Условное положение главного золотника
    // с учётом зазора
    main_z_pos = cut(main_z_pos,
                     disjunction_z_pos - (main_z_eps / 2.0),
                     disjunction_z_pos + (main_z_eps / 2.0));

    // Поток из тормозной магистрали в запасный резервуар
    double Q_bp_sr = 0.0;
    // Проверяем связь запасного резезвуара и тормозной магистрали
    // через три отверстия диаметром 1.25 мм
    if (disjunction_z_pos < p[1])
    {
        // Коэффициент перетока через зазор между пояском поршня и втулкой
        double K_2 = cut(A[1] * (disjunction_z_pos - p[2]), 0.0, K[2]);
        // Поток из тормозной магистрали в запасный резервуар
        Q_bp_sr = (K[1] + K_2) * (pBP - pSR);
    }

    // Потоки камеры дополнительной разрядки ТМ
    double Q_bp_atm = 0.0;
    double Q_bp_kd = 0.0;
    double Q_kd_atm = 0.0;
    // Потоки магистрали тормозных цилиндров
    double Q_sr_bc = 0.0;
    double Q_bc_atm = 0.0;

    // Проверяем, что главный золотник не в отпускном положении
    // (управляющие КДР и ТЦ каналы золотника совпадают с зеркалом)
    if (main_z_pos > p[3])
    {
        // Проверяем, что главный золотник не в экстренном положении
        if (main_z_pos < p[4])
        {
            // Проверяем взаимное положение золотников
            double z_diff = A[2] * (disjunction_z_pos - main_z_pos - p[5]);

            // Поток из тормозной магистрали в камеру дополнительной разрядки ТМ
            Q_bp_kd = K[3] * cut(z_diff, 0.0, 1.0) * (pBP - Y[KDR]);
            // Поток из камеры дополнительной разрядки ТМ в атмосферу
            Q_kd_atm = K[4] * cut(-z_diff, 0.0, 1.0) * Y[KDR];

            // Поток из запасного резервуара в магистраль тормозных цилиндров
            Q_sr_bc = K[5] * cut(z_diff, 0.0, 1.0) * (pSR - pBC);
        }
        else
        {
            switch (long_train_mode) {
            case 0:
            {
                // Открытие срывного клапана экстренного торможения
                double v_emerg = cut(A[0] * (pBP - pBC - p[0]), 0.0, 1.0);
                // Поток экстренной разрядки тормозной магистрали
                Q_bp_atm = K[0] * v_emerg * pBP;

                // Поток из запасного резервуара в магистраль тормозных цилиндров
                Q_sr_bc = K[6] * (pSR - pBC);
                break;
            }
            case 1:
            {
                // Открытие срывного клапана экстренного торможения
                double v_emerg = cut(A[0] * (pBP - pBC - p[0]), 0.0, 1.0);
                // Поток экстренной разрядки тормозной магистрали
                Q_bp_atm = K[0] * v_emerg * pBP;

                // Поток из запасного резервуара в магистраль тормозных цилиндров
                Q_sr_bc = K[7] * (pSR - pBC);
                break;
            }
            case 2:
            default:
            {
                // Поток из запасного резервуара в магистраль тормозных цилиндров
                Q_sr_bc = K[7] * (pSR - pBC);
                break;
            }
            }

            // Поток из камеры дополнительной разрядки ТМ в атмосферу
            Q_kd_atm = K[4] * Y[KDR];
        }
    }
    else
    {
        // Разрядка магистрали тормозных цилиндров в атмосферу
        Q_bc_atm = K[8] * pBC;
        // Поток из камеры дополнительной разрядки ТМ в атмосферу
        Q_kd_atm = K[4] * Y[KDR];
    }

/*
    TODO протестировать и настроить всё
*/
    // Поток в тормозную магистраль
    QBP = - Q_bp_sr - Q_bp_kd - Q_bp_atm;

    // Поток в магистраль тормозных цилиндров
    QBC = Q_sr_bc - Q_bc_atm;

    // Поток в запасный резервуар
    QSR = Q_bp_sr - Q_sr_bc;

    // Поток в камеру дополнительной разрядки ТМ
    Qkd = Q_bp_kd - Q_kd_atm;

//    QString("  time  ; pBP   ; pBC   ; pSR   ; pKDR  ; BPsr   ; BPkdr  ; KDRatm ; SRbc   ; BCatm  ");
    DebugMsg = QString("%1;%2;%3;%4;%5;%6;%7;%8;%9;%10")
            .arg(t, 8, 'f', 3)
            .arg(10*pBP, 7, 'f', 5)
            .arg(10*pBC, 7, 'f', 5)         //%3
            .arg(10*pSR, 7, 'f', 5)
            .arg(10*Y[KDR], 7, 'f', 5)
            .arg(10000*Q_bp_sr, 8, 'f', 5)  //%6
            .arg(10000*Q_bp_kd, 8, 'f', 5)
            .arg(10000*Q_kd_atm, 8, 'f', 5)
            .arg(10000*Q_sr_bc, 8, 'f', 5)  //%9
            .arg(10000*Q_bc_atm, 8, 'f', 5);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AirDist292::ode_system(const state_vector_t &Y,
                            state_vector_t &dYdt,
                            double t)
{
    Q_UNUSED(t)
    Q_UNUSED(Y)
    dYdt[KDR] = Qkd / Vkd;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AirDist292::load_config(CfgReader &cfg)
{
    QString secName = "Device";

    cfg.getInt(secName, "LongTrainMode", long_train_mode);

    double tmp = 0.0;
    cfg.getDouble(secName, "Vkd", tmp);
    if (tmp > 1e-3)
        Vkd = tmp;

    cfg.getDouble(secName, "main_z_eps", main_z_eps);

    for (size_t i = 0; i < p.size(); ++i)
    {
        QString coeff = QString("p%1").arg(i);
        cfg.getDouble(secName, coeff, p[i]);
    }

    for (size_t i = 0; i < K.size(); ++i)
    {
        QString coeff = QString("K%1").arg(i);
        cfg.getDouble(secName, coeff, K[i]);
    }

    for (size_t i = 0; i < A.size(); ++i)
    {
        QString coeff = QString("A%1").arg(i);
        cfg.getDouble(secName, coeff, A[i]);
    }

}

GET_AIR_DISTRIBUTOR(AirDist292)
