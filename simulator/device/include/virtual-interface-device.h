#ifndef     VIRTUAL_INTERFACE_DEVICE_H
#define     VIRTUAL_INTERFACE_DEVICE_H

#include    <QObject>
#include    "device-export.h"

#include    "control-signals.h"
#include    "feedback-signals.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DEVICE_EXPORT VirtualInterfaceDevice : public QObject
{
    Q_OBJECT

public:

    VirtualInterfaceDevice(QObject *parent = Q_NULLPTR);

    virtual ~VirtualInterfaceDevice();

    virtual bool init(QString cfg_path) = 0;

    virtual void process() = 0;

    signal_t getControlSignal(size_t id);

signals:

    void sendControlSignals(control_signals_t control_signals);

public slots:

    void receiveFeedback(feedback_signals_t feedback_signals);

protected:

    control_signals_t   control_signals;

    feedback_signals_t  feedback_signals;
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
typedef VirtualInterfaceDevice* (*GetInterfaceDevice)();

#define GET_INTERFACE_DEVIC(ClassName) \
    extern "C" Q_DECL_EXPORT VirtualInterfaceDevice *getInterfaceDevice() \
    { \
        return new (ClassName) (); \
    }

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
extern "C" Q_DECL_EXPORT VirtualInterfaceDevice *loadInterfaceDevice(QString lib_path);

#endif // VIRTUAL_INTERFACE_DEVICE_H
