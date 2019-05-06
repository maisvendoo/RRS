#include    "automatic-train-stop.h"

#include    <QLibrary>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
AutoTrainStop::AutoTrainStop(QObject *parent) : BrakeDevice(parent)
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
