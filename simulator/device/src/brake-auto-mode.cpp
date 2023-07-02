#include    "brake-auto-mode.h"

#include    <QLibrary>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
BrakeAutoMode::BrakeAutoMode(QObject *parent) : BrakeDevice(parent)
  , payload_coeff(0.0)
  , pBC(0.0)
  , QadBC(0.0)
  , QBC(0.0)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BrakeAutoMode::setAirDistBCflow(double value)
{
    QadBC = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double BrakeAutoMode::getAirDistBCpressure() const
{
    return getY(AUTO_MODE_WORK_PRESSURE);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BrakeAutoMode::setBCpressure(double value)
{
    pBC = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double BrakeAutoMode::getBCflow() const
{
    return QBC;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
BrakeAutoMode::~BrakeAutoMode()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
BrakeAutoMode *loadBrakeAutoMode(QString lib_path)
{
    BrakeAutoMode *automode = nullptr;

    QLibrary lib(lib_path);

    if (lib.load())
    {
        GetBrakeAutoMode getBrakeAutoMode = reinterpret_cast<GetBrakeAutoMode>(lib.resolve("getBrakeAutoMode"));

        if (getBrakeAutoMode)
        {
            automode = getBrakeAutoMode();
        }
    }

    return automode;
}
