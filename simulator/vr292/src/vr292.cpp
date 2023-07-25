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
    disjunction_z_pos = pSR - pBP;

    // Условное положение главного золотника
    main_z_pos = cut(main_z_pos,
                     disjunction_z_pos - main_z_eps,
                     disjunction_z_pos);

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
/*
    TODO
*/
    // Поток в тормозную магистраль
    QBP = -Q_bp_sr;

    // Поток в магистраль тормозных цилиндров
    QBC = 0.0;

    // Поток в запасный резервуар
    QSR = Q_bp_sr;

    // Поток в камеру дополнительной разрядки ТМ
    Qkd = 0.0;
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
    dYdt[0] = Qkd / Vkd;
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

    for (size_t i = 1; i < p.size(); ++i)
    {
        QString coeff = QString("p%1").arg(i);
        cfg.getDouble(secName, coeff, p[i]);
    }

    for (size_t i = 1; i < K.size(); ++i)
    {
        QString coeff = QString("K%1").arg(i);
        cfg.getDouble(secName, coeff, K[i]);
    }

    for (size_t i = 1; i < A.size(); ++i)
    {
        QString coeff = QString("A%1").arg(i);
        cfg.getDouble(secName, coeff, A[i]);
    }

}

GET_AIR_DISTRIBUTOR(AirDist292)
