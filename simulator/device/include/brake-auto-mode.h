#ifndef     BRAKE_AUTOMODE_H
#define     BRAKE_AUTOMODE_H

#include    "brake-device.h"

#include    "physics.h"

static constexpr int AUTO_MODE_WORK_PRESSURE = 0; ///< Давление в рабочей камере авторежима

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DEVICE_EXPORT BrakeAutoMode : public BrakeDevice
{
public:

    BrakeAutoMode(QObject* parent = nullptr);

    virtual ~BrakeAutoMode() = default;

    /// Задать уровень сжатия штока авторежима (уровень загрузки вагона), 0..1
    void setPayloadCoeff(double value) { payload_coeff = std::clamp(value, 0.0, 1.0); }

    /// Задать поток от воздухораспределителя
    void setAirDistBCflow(double value) noexcept;

    /// Давление в рабочей камере авторежима (имитирует давление в тормозных цилиндрах)
    double getAirDistBCpressure() const;

    /// Задать давление от магистрали тормозных цилиндров (или импульсной магистрали)
    void setBCpressure(double value) noexcept;

    /// Поток в магистраль тормозных цилиндров (или в импульсную магистраль)
    double getBCflow() const;

protected:

    /// Уровень сжатия штока авторежима (уровень загрузки вагона), 0..1
    double  payload_coeff = 0.0;

    /// Давление в тормозном цилиндре, MPa
    double  pBC = 0.0;

    /// Поток от воздухораспределителя для управления тормозными цилиндрами
    double  QadBC = 0.0;

    /// Поток в магистраль тормозных цилиндров
    double  QBC = 0.0;
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
using GetBrakeAutoMode = BrakeAutoMode*(*)();

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
#define GET_BRAKE_AUTOMODE(ClassName) \
    extern "C" BrakeAutoMode* getBrakeAutoMode() \
    { \
        return new (ClassName) (); \
    }

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
extern "C" DEVICE_EXPORT BrakeAutoMode* loadBrakeAutoMode(QString lib_path);

#endif // BRAKE_AUTOMODE_H
