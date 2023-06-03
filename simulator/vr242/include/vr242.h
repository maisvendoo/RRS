#ifndef		VR242_H
#define		VR242_H

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
class AirDist242 : public AirDistributor
{
public:

    AirDist242();

    ~AirDist242();

    void init(double pBP, double pFL);

private:

    /// Признак длинносоставного режима
    bool long_train_mode;

    /// Признак включения ускорителя экстренного торможения
    bool emergency_mode;

    /// Объем ускорительной камеры
    double Vuk;

    /// Поток в ускорительную камеру
    double Quk;

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

    std::array<double, MAX_FLOW_COEFFS> K;
    std::array<double, MAX_GIAN_COEFFS> k;

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);

    void preStep(state_vector_t &Y, double t);
};

#endif // VR242_H
