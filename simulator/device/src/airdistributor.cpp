#include    "airdistributor.h"

#include    <QLibrary>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
AirDistributor::AirDistributor(QObject* parent) : BrakeDevice(parent)
{
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AirDistributor::setBPpressure(double value) noexcept
{
    pBP = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double AirDistributor::getBPflow() const noexcept
{
    return QBP;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AirDistributor::setBCpressure(double value) noexcept
{
    pBC = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double AirDistributor::getBCflow() const noexcept
{
    return QBC;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AirDistributor::setSRpressure(double value) noexcept
{
    pSR = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double AirDistributor::getSRflow() const noexcept
{
    return QSR;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
AirDistributor* loadAirDistributor(QString lib_path)
{
    AirDistributor* airdist = nullptr;

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
