#include    "automatic-train-stop.h"

#include    <QLibrary>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
AutoTrainStop::AutoTrainStop(QObject* parent) : BrakeDevice(parent)
{
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AutoTrainStop::setKeyOn(bool state) noexcept
{
    is_key_on = static_cast<double>(state);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool AutoTrainStop::isKeyOn() const noexcept
{
    return static_cast<bool>(is_key_on);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AutoTrainStop::setPowered(bool state) noexcept
{
    is_powered = static_cast<double>(state);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool AutoTrainStop::isPowered() const noexcept
{
    return static_cast<bool>(is_powered);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool AutoTrainStop::getEmergencyBrakeContact() const
{
    return false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AutoTrainStop::setFLpressure(double value) noexcept
{
    pFL = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double AutoTrainStop::getFLflow() const noexcept
{
    return QFL;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AutoTrainStop::setBPpressure(double value) noexcept
{
    pBP = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double AutoTrainStop::getBPflow() const noexcept
{
    return QBP;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
AutoTrainStop* loadAutoTrainStop(QString lib_path)
{
    AutoTrainStop* autostop = nullptr;

    QLibrary lib(lib_path);

    if (lib.load())
    {
        GetAutoTrainStop getAutoTrainStop = reinterpret_cast<GetAutoTrainStop>(lib.resolve("getAutoTrainStop"));

        if (getAutoTrainStop)
        {
            autostop = getAutoTrainStop();
        }
    }

    return autostop;
}
