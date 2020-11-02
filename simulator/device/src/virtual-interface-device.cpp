#include    "virtual-interface-device.h"

#include    <QLibrary>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
VirtualInterfaceDevice::VirtualInterfaceDevice(QObject *parent)
    : QObject(parent)
    , cfg_dir("")
{
    qRegisterMetaType<signal_t>();
    qRegisterMetaType<control_signals_t>();
    qRegisterMetaType<feedback_signals_t>();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
VirtualInterfaceDevice::~VirtualInterfaceDevice()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
signal_t VirtualInterfaceDevice::getControlSignal(size_t id)
{
    return control_signals.analogSignal[id];
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString VirtualInterfaceDevice::getConfigDirectoryName() const
{
    return cfg_dir;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VirtualInterfaceDevice::receiveFeedback(feedback_signals_t feedback_signals)
{
    this->feedback_signals = feedback_signals;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
VirtualInterfaceDevice *loadInterfaceDevice(QString lib_path)
{
    VirtualInterfaceDevice *device = nullptr;

    QLibrary lib(lib_path);

    if (lib.load())
    {
        GetInterfaceDevice getInterfaceDevice = reinterpret_cast<GetInterfaceDevice>(lib.resolve("getInterfaceDevice"));

        if (getInterfaceDevice)
        {
            device = getInterfaceDevice();
        }
    }

    return device;
}
