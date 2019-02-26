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

private:

    double Vmk;
    double Vy4;
    double Vzk;
    double A1;

    std::array<double, MAX_FLOW_COEFFS> K;
    std::array<double, MAX_GIAN_COEFFS> k;

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);

    void preStep(const state_vector_t &Y, double t);
};

#endif // VR242
