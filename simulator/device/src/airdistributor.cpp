#include    "airdistributor.h"

#include    <QLibrary>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
AirDistributor::AirDistributor(QObject *parent) : Device(parent)
  , pTM(0.0)
  , pBC(0.0)
  , Qbc(0.0)
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
double AirDistributor::getBrakeCylinderAirFlow() const
{
    return Qbc;
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
