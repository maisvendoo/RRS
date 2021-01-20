#include    "automatic-train-stop.h"

#include    <QLibrary>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
AutoTrainStop::AutoTrainStop(QObject *parent) : BrakeDevice(parent)
  , is_powered(0.0)
  , is_key_on(0.0)
  , pFL(0.0)
  , pTM(0.0)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
AutoTrainStop::~AutoTrainStop()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AutoTrainStop::powerOn(bool on)
{
    is_powered = static_cast<double>(on);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AutoTrainStop::keyOn(bool on)
{
    is_key_on = static_cast<double>(on);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AutoTrainStop::setFeedlinePressure(double pFL)
{
    this->pFL = pFL;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AutoTrainStop::setBrakepipePressure(double pTM)
{
    this->pTM = pTM;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double AutoTrainStop::getEmergencyBrakeRate() const
{
    return 0.0;
}

bool AutoTrainStop::getState()
{
    return static_cast<bool>(is_key_on);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
AutoTrainStop *loadAutoTrainStop(QString lib_path)
{
    AutoTrainStop *autostop = nullptr;

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
