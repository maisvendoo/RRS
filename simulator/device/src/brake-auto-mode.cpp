#include    "brake-auto-mode.h"

#include    <QLibrary>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
BrakeAutoMode::BrakeAutoMode(QObject* parent) : BrakeDevice(parent)
{
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BrakeAutoMode::setAirDistBCflow(double value) noexcept
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
void BrakeAutoMode::setBCpressure(double value) noexcept
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
