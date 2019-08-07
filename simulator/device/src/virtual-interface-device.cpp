#include    "virtual-interface-device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
VirtualInterfaceDevice::VirtualInterfaceDevice(QObject *parent)
    : QObject(parent)
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
void VirtualInterfaceDevice::receiveFeedback(feedback_signals_t feedback_signals)
{
    this->feedback_signals = feedback_signals;
}
