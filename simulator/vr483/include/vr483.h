#ifndef	    VR483
#define	    VR483

#include    "airdistributor.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
enum
{
    NUM_VOLUMES = 3,
    NUM_COEFFS = 14,
    NUM_PRESSURES = 18,

    RK = 0, ///< Давление в рабочей камере
    ZK = 1, ///< Давление в золотниковой камере
    KDR = 2 ///< Давление в каналах дополнительной разрядки
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class AirDist483 : public AirDistributor
{
public:

    AirDist483();

    ~AirDist483();

    void init(double pBP, double pFL);

private:

    /// Переключатель режима профиля пути:
    /// 0 - горный; 1 - равнинный
    int switchProfile;

    /// Переключатель режима загруженности вагона:
    /// 0 - П (порожний); 1 - С (средний); 2 - Г (гружёный)
    int switchPayload;

    /// Объёмы камер, м^3
    std::array<double, NUM_VOLUMES> v;
    /// Расход воздуха в камеры
    std::array<double, NUM_VOLUMES> Q;

    /// Коэффициенты перетока воздуха
    std::array<double, NUM_COEFFS> k;

    /// Давления открытия клапанов, начала перемещения поршней и т.д., МПа
    std::array<double, NUM_PRESSURES> p;

    /// Коэффициент чувствительности к разности сил на уравнительном поршне
    double A1;

    double A2;

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);

    void preStep(state_vector_t &Y, double t);
};

#endif // VR483
