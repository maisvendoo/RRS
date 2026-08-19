#ifndef		VR242_H
#define		VR242_H

#include	"airdistributor.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class AirDist242 : public AirDistributor
{
public:

    AirDist242();

    ~AirDist242() = default;

    void init(double pBP, double pFL) override;

#ifndef NDEBUG
    QString getDebugMsg() const override;
#endif

private:

    enum
    {
        MAX_FLOW_COEFFS = 10,
        MAX_SENS_COEFFS = 5,

        UK = 0 ///< Y[0] - Давление в ускорительной камере
    };

    /// Признак длинносоставного режима
    bool long_train_mode = false;

    /// Признак включения ускорителя экстренного торможения
    bool emergency_mode = false;

    /// Объем ускорительной камеры
    double Vuk = 1.0e-3;

    /// Поток в ускорительную камеру
    double Quk = 0.0;

    /// Давление срабатывания клапана в камере У2 (разобщение ТМ и ЗР)
    double pu2 = 0.039;

    /// Давление запирания тормозного клапана в длинносоставном режиме
    double pbv = 0.15;

    /// Давление открытия срывного клапана ускорения экстренного торможения
    double psv = 0.025;

    /// Давление открытия широкого канала в ускорительную камеру (сверхзарядное)
    double pwv = 0.48;

    double s1_min = -0.015;
    double s1_max = 0.015;

    std::array<double, MAX_FLOW_COEFFS> K;
    std::array<double, MAX_SENS_COEFFS> A;

#ifndef NDEBUG
    mutable bool is_upd = false;
    QString DebugMsg = "";
#endif

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t) override;

    void load_config(CfgReader &cfg) override;

    void preStep(state_vector_t &Y, double t) override;
};

#endif // VR242_H
