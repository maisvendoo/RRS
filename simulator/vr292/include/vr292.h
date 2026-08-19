#ifndef		VR292_H
#define		VR292_H

#include	"airdistributor.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class AirDist292 : public AirDistributor
{
public:

    AirDist292();

    ~AirDist292() = default;

    void init(double pBP, double pFL) override;

#ifndef NDEBUG
    QString getDebugMsg() const override;
#endif

private:

    enum
    {
        NUM_PRESSURES = 6,
        NUM_FLOW_COEFFS = 9,
        NUM_SENSITIVITY_COEFFS = 3,

        KDR = 0 ///< Давление в камере дополнительной разрядки ТМ
    };

    /// Признак длинносоставного режима:
    /// 0 - короткосоставный (К), 1 - длинносоставный (Д),
    /// 2 - ускоритель экстренного выключен (УВ)
    int long_train_mode = 0;

    /// Объем камеры дополнительной разрядки ТМ
    double Vkd = 1.0e-3;
    /// Поток в камеру дополнительной разрядки ТМ
    double Qkd = 0.0;

    /// Условное положение магистрального поршня и отсекательного золотника
    double disjunction_z_pos = 0.0;
    /// Задержка магистрального поршня к равновесию давлений из-за трения
    double disjunction_z_eps = 0.03;

    /// Условное положение главного золотника
    double main_z_pos = 0.0;
    /// Условный зазор главного золотника
    /// (холостой ход магистрального поршня без перемещения золотника)
    double main_z_eps = 0.015;

    std::array<double, NUM_PRESSURES> p;
    std::array<double, NUM_FLOW_COEFFS> K;
    std::array<double, NUM_SENSITIVITY_COEFFS> A;

#ifndef NDEBUG
    mutable bool is_upd = false;
    QString DebugMsg = "";
#endif

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t) override;

    void load_config(CfgReader &cfg) override;

    void preStep(state_vector_t &Y, double t) override;
};

#endif // VR292_H
