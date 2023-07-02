#ifndef     BRAKE_AUTOMODE_H
#define     BRAKE_AUTOMODE_H

#include    "brake-device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
enum
{
    AUTO_MODE_WORK_PRESSURE = 0 ///< Давление в рабочей камере авторежима
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DEVICE_EXPORT BrakeAutoMode : public BrakeDevice
{
public:

    BrakeAutoMode(QObject *parent = Q_NULLPTR);

    virtual ~BrakeAutoMode();

    /// Задать уровень сжатия штока авторежима (уровень загрузки вагона), 0..1
    void setPayloadCoeff(double value) { payload_coeff = cut(value, 0.0, 1.0); }

    /// Задать поток от воздухораспределителя
    void setAirDistBCflow(double value);

    /// Давление в рабочей камере авторежима (имитирует давление в тормозных цилиндрах)
    double getAirDistBCpressure() const;

    /// Задать давление от магистрали тормозных цилиндров (или импульсной магистрали)
    void setBCpressure(double value);

    /// Поток в магистраль тормозных цилиндров (или в импульсную магистраль)
    double getBCflow() const;

protected:

    /// Уровень сжатия штока авторежима (уровень загрузки вагона), 0..1
    double  payload_coeff;

    /// Давление в тормозном цилиндре, MPa
    double  pBC;

    /// Поток от воздухораспределителя для управления тормозными цилиндрами
    double  QadBC;

    /// Поток в магистраль тормозных цилиндров
    double  QBC;
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
typedef BrakeAutoMode* (*GetBrakeAutoMode)();

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
#define GET_BRAKE_AUTOMODE(ClassName) \
    extern "C" Q_DECL_EXPORT BrakeAutoMode *getBrakeAutoMode() \
    { \
        return new (ClassName) (); \
    }

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
extern "C" Q_DECL_EXPORT BrakeAutoMode *loadBrakeAutoMode(QString lib_path);

#endif // BRAKE_AUTOMODE_H
