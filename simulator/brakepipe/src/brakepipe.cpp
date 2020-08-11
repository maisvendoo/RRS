//------------------------------------------------------------------------------
//
//      Train's brakepipe model
//      (с) maisvendoo, 06/11/2016
//      Developer: Dmitry Pritykin
//
//------------------------------------------------------------------------------
/*!
 *  \file
 *  \brief Train's brakepipe model
 *  \copyright maisvendoo
 *  \author Dmitry Pritykin
 *  \date 06/11/2016
 */

#include "brakepipe.h"
#include "CfgReader.h"
#include "sweep.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
BrakePipe::BrakePipe()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
BrakePipe::~BrakePipe()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BrakePipe::setLength(double L)
{
    this->L = L;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BrakePipe::setNodesNum(size_t N)
{
    // Set nodes count
    this->N = N;

    // Share memory
    p.resize(N + 2);
    V.resize(N + 2);

    A.resize(N + 1);
    B.resize(N + 1);
    C.resize(N + 1);
    f.resize(N + 1);

    // Set length step
    h = L / (N + 1);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BrakePipe::setBeginPressure(double p0)
{
    p[0] = p0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BrakePipe::setAuxRate(size_t i, double auxRate)
{
    V[i] = auxRate;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool BrakePipe::step(double t, double dt)
{
    Q_UNUSED(t)

    // Implicit method of parabolic PDE solution

    // Calculation of coefficients
    for (size_t i = 0; i < N; i++)
    {
        A[i] = -c0 / h / h;
        B[i] = A[i];

        C[i] = 1 / dt + 2 * c0 / h / h;

        f[i] = -Q(i) - V[i + 1] + p[i + 1] / dt;
    }

    A[0] = 0;
    A[N] = -c0 / h / h;

    B[N] = 0;

    C[N] = 1 / dt + c0 / h / h;

    f[0] += c0 * p[0] / h / h;
    f[N] = -Q(N) - V[N] + p[N] / dt;

    // Linear equation system solution
    sweep(N + 1, A.data(), B.data(), C.data(), f.data(), p.data() + 1);

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool BrakePipe::init(QString cfg_path)
{
    // Loading of config params
    loadCfg(cfg_path);

    // Model parameters calculation
    double R = Physics::Rmu / Physics::Mair;

    a = 0.25*sqrt( (5.5e5 - Physics::pA)*R*T*lambda/d/L/(5.5e5 + Physics::pA) );
    c0 = Physics::c * Physics::c /2 / a;
    a1 = 0.1265*muf*muf*lambda / pow(d, 5);

    // Initial pressure distribution
    for (size_t i = 1; i < N + 1; i++)
    {
        double x = i*h;
        p[i] = p[0]*exp(-a1*(pow(L, 3) - pow(L - x, 3)));

        if (p[i] < Physics::pA) p[i] = Physics::pA;

        V[i] = 0;
    }

    p[N + 1] = p[0]*exp(-a1*pow(L, 3));

    if (p[N + 1] < Physics::pA) p[N+1] = Physics::pA;

    V[N + 1] = 0;

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double BrakePipe::getPressure(size_t i)
{
    double press = (p[i] - Physics::pA) / Physics::MPa;

    if (press < 0)
        press = 0;

    return press;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double BrakePipe::Q(size_t i)
{
    double x = i*h;

    return c0*a1*p[0]*( 9*a1*pow(L - x, 4) + 6*(L - x) ) * exp( -a1*(pow(L, 3) - pow(L - x, 3) ) );
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool BrakePipe::loadCfg(QString cfg_path)
{
    CfgReader cfg;


    if (cfg.load(cfg_path))
    {
        if (!cfg.getDouble("BrakePipe", "lambda", lambda))
        {
            lambda = 0.066;
        }

        if (!cfg.getDouble("BrakePipe", "diameter", d))
        {
            d = 0.0343;
        }

        if (!cfg.getDouble("BrakePipe", "muf", muf))
        {
            muf = 1.5e-8;
        }

        double t = 0;
        if (!cfg.getDouble("BrakePipe", "AirTemp", t))
        {
            t = 20.0;
        }

        T = 273 + t;
    }
    else
    {
        lambda = 0.066;
        d = 0.0343;
        muf = 1.5e-8;
        T = 293;
    }

    return true;
}
