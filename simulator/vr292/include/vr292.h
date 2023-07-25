#ifndef		VR292_H
#define		VR292_H

#include	"airdistributor.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
enum
{
    NUM_PRESSURES = 10,
    NUM_FLOW_COEFFS = 10,
    NUM_SENSITIVITY_COEFFS = 10
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

    /// Условное положение магистрального поршня и отсекательного золотника
    double disjunction_z_pos;
    /// Условное положение главного золотника
    double main_z_pos;
    /// Условный зазор главного золотника
    /// (холостой ход магистрального поршня без перемещения золотника)
    double main_z_eps;

    std::array<double, NUM_PRESSURES> p;
    std::array<double, NUM_FLOW_COEFFS> K;
    std::array<double, NUM_SENSITIVITY_COEFFS> A;

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);

    void preStep(state_vector_t &Y, double t);
};

#endif // VR292_H
