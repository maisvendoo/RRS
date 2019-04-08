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

    void init(double pTM);

private:

    double Vmk;
    double Vy4;
    double Vzk;
    double A1;
    double Vat2;
    double Vy2;
    double py2;

    double s1_min;
    double s1_max;

    double p_bv;
    double p_UP;
    double long_train;

    std::array<double, MAX_FLOW_COEFFS> K;
    std::array<double, MAX_GIAN_COEFFS> k;
    std::array<double, MAX_TIME_CONST> T;

    double K2;

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);

    void preStep(state_vector_t &Y, double t);
};

#endif // VR242
