#ifndef     AIR_DISTRIBUTOR_H
#define     AIR_DISTRIBUTOR_H

#include    "brake-device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DEVICE_EXPORT AirDistributor : public BrakeDevice
{
public:

    AirDistributor(QObject* parent = nullptr);

    virtual ~AirDistributor() noexcept = default;

    /// Задать давление от тормозной магистрали
    void setBPpressure(double value) noexcept;

    /// Поток в тормозную магистраль
    double getBPflow() const noexcept;

    /// Задать давление от магистрали тормозных цилиндров (или импульсной магистрали)
    void setBCpressure(double value) noexcept;

    /// Поток в магистраль тормозных цилиндров (или в импульсную магистраль)
    double getBCflow() const noexcept;

    /// Задать давление от запасного резервуара
    void setSRpressure(double value) noexcept;

    /// Поток в запасный резервуар
    double getSRflow() const noexcept;

protected:
    double pBP = 0.0;
    double pBC = 0.0;
    double pSR = 0.0;

    double QBP = 0.0;
    double QBC = 0.0;
    double QSR = 0.0;
};

#endif // AIR_DISTRIBUTOR_H
