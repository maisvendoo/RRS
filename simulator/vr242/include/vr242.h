#ifndef		VR242
#define		VR242

#include	"airdistributor.h"
#include    "constants.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class AirDist242 : public AirDistributor
{
public:

    AirDist242();

    ~AirDist242();

    void init(double pTM, double pFL);

private:

    double A1;
    double py2;

    double s1_min;
    double s1_max;

    double p_bv;
    double p_UP;
    double long_train;
    double K2;

    std::array<double, MAX_FLOW_COEFFS> K;
    std::array<double, MAX_GIAN_COEFFS> k;
    std::array<double, MAX_TIME_CONST> T;

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);

    void preStep(state_vector_t &Y, double t);
};

#endif // VR242
