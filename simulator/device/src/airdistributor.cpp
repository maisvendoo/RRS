#include    "airdistributor.h"

#include    <QLibrary>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
AirDistributor::AirDistributor(QObject *parent) : Device(parent)
  , pTM(0.0)
  , pBC(0.0)
  , pAS(0.0)
  , Qbc(0.0)
  , Qas(0.0)
  , auxRate(0.0)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
AirDistributor::~AirDistributor()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AirDistributor::setBrakepipePressure(double pTM)
{
    this->pTM = pTM;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AirDistributor::setBrakeCylinderPressure(double pBC)
{
    this->pBC = pBC;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AirDistributor::setAirSupplyPressure(double pAS)
{
    this->pAS = pAS;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double AirDistributor::getBrakeCylinderAirFlow() const
{
    return Qbc;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double AirDistributor::getAirSupplyFlow() const
{
    return Qas;
}

double AirDistributor::getAuxRate() const
{
    return auxRate;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
AirDistributor *loadAirDistributor(QString lib_path)
{
    AirDistributor *airdist = nullptr;

    QLibrary lib(lib_path);

    if (lib.load())
    {
        GetAirDistributor getAirDistributor = reinterpret_cast<GetAirDistributor>(lib.resolve("getAirDistributor"));

        if (getAirDistributor)
        {
            airdist = getAirDistributor();
        }
    }

    return airdist;
}
