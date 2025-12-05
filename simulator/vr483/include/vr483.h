#ifndef	    VR483
#define	    VR483

#include    "airdistributor.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class AirDist483 : public AirDistributor
{
public:

    AirDist483();

    ~AirDist483() = default;

    void init(double pBP, double pFL) override;

#ifndef NDEBUG
    QString getDebugMsg() const override;
#endif

private:

    enum
    {
        NUM_VOLUMES = 3,
        NUM_COEFFS = 14,
        NUM_SENSIVITY_COEFFS = 9,
        NUM_PRESSURES = 18,

        RK = 0, ///< Y[0] - Давление в рабочей камере
        ZK = 1, ///< Y[1] - Давление в золотниковой камере
        KDR = 2 ///< Y[2] - Давление в каналах дополнительной разрядки
    };

    /// Переключатель режима профиля пути:
    /// 0 - горный; 1 - равнинный
    int switchProfile = 1;

    /// Переключатель режима загруженности вагона:
    /// 0 - П (порожний); 1 - С (средний); 2 - Г (гружёный)
    int switchPayload = 1;

    /// Объёмы камер, м^3
    std::array<double, NUM_VOLUMES> v = {0.006, 0.0045, 0.0005};
    /// Расход воздуха в камеры
    std::array<double, NUM_VOLUMES> Q = {0.0, 0.0, 0.0};

    /// Коэффициенты перетока воздуха
    std::array<double, NUM_COEFFS> k;

    /// Давления открытия клапанов, начала перемещения поршней и т.д., МПа
    std::array<double, NUM_PRESSURES> p;

    /// Коэффициенты чувствительности для дросселирования клапанов
    std::array<double, NUM_SENSIVITY_COEFFS> A;

#ifndef NDEBUG
    mutable bool is_upd = false;
    QString DebugMsg = "";
#endif

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t) override;

    void load_config(CfgReader &cfg) override;

    void preStep(state_vector_t &Y, double t) override;
};

#endif // VR483
