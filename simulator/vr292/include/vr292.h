#ifndef		VR292_H
#define		VR292_H

#include	"airdistributor.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
enum
{
    MAX_FLOW_COEFFS = 10,
    MAX_GIAN_COEFFS = 3,
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class AirDist292 : public AirDistributor
{
public:

    AirDist292();

    ~AirDist292();

    void init(double pBP, double pFL);

private:

    /// Признак длинносоставного режима:
    /// 0 - короткосоставный (К), 1 - длинносоставный (Д),
    /// 2 - ускоритель экстренного выключен (УВ)
    int long_train_mode;

    /// Объем камеры дополнительной разрядки ТМ
    double Vkd;

    /// Поток в камеру дополнительной разрядки ТМ
    double Qkd;
/*
    /// Давление срабатывания клапана в камере У2 (разобщение ТМ и ЗР)
    double pu2;

    /// Давление запирания тормозного клапана в длинносоставном режиме
    double pbv;

    /// Давление открытия срывного клапана ускорения экстренного торможения
    double psv;

    /// Давление открытия широкого канала в ускорительную камеру (сверхзарядное)
    double pwv;

    double s1_min;
    double s1_max;
*/
    std::array<double, MAX_FLOW_COEFFS> K;
    std::array<double, MAX_GIAN_COEFFS> k;

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);

    void preStep(state_vector_t &Y, double t);
};

#endif // VR292_H
