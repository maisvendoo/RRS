#ifndef     AIR_DISTRIBUTOR_H
#define     AIR_DISTRIBUTOR_H

#include    "brake-device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DEVICE_EXPORT AirDistributor : public BrakeDevice
{
public:

    AirDistributor(QObject *parent = Q_NULLPTR);

    virtual ~AirDistributor();

    /// Задать давление от тормозной магистрали
    void setBPpressure(double value);

    /// Поток в тормозную магистраль
    double getBPflow() const;

    /// Задать давление от магистрали тормозных цилиндров (или импульсной магистрали)
    void setBCpressure(double value);

    /// Поток в магистраль тормозных цилиндров (или в импульсную магистраль)
    double getBCflow() const;

    /// Задать давление от запасного резервуара
    void setSRpressure(double value);

    /// Поток в запасный резервуар
    double getSRflow() const;

protected:

    double pBP;
    double pBC;
    double pSR;

    double QBP;
    double QBC;
    double QSR;
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
typedef AirDistributor* (*GetAirDistributor)();

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
#define GET_AIR_DISTRIBUTOR(ClassName) \
    extern "C" Q_DECL_EXPORT AirDistributor *getAirDistributor() \
    { \
        return new (ClassName) (); \
    }

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
extern "C" Q_DECL_EXPORT AirDistributor *loadAirDistributor(QString lib_path);

#endif // AIR_DISTRIBUTOR_H
