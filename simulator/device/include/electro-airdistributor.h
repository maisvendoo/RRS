#ifndef     ELECTRO_AIRDISTRIBUTOR_H
#define     ELECTRO_AIRDISTRIBUTOR_H

#include    "brake-device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DEVICE_EXPORT ElectroAirDistributor : public BrakeDevice
{
public:

    ElectroAirDistributor(QObject *parent = Q_NULLPTR);

    ~ElectroAirDistributor();

    void setControlLinesNumber(size_t num);

    void setValvesNumber(size_t num);

    void setControlLine(double value, size_t idx = 0);

    double getValveState(size_t i);

    // Взаимодействие оборудования, подключаемого через данный электровоздухораспределитель
    // Взаимодействие воздухораспределителя и магистрали тормозных цилиндров
    /// Задать давление от магистрали тормозных цилиндров
    void setBCpressure(double value);

    /// Давление магистрали тормозных цилиндров к воздухораспределителю
    double getAirdistBCpressure() const;

    /// Задать поток из воздухораспределителя в магистраль тормозных цилиндров
    void setAirdistBCflow(double value);

    /// Поток в магистраль тормозных цилиндров
    double getBCflow() const;

    // Взаимодействие воздухораспределителя и запасного резервуара
    /// Задать давление от запасного резервуара
    void setSRpressure(double value);

    /// Давление запасного резервуара к воздухораспределителю
    double getAirdistSRpressure() const;

    /// Задать поток из воздухораспределителя в запасный резервуар
    void setAirdistSRflow(double value);

    /// Поток в запасный резервуар
    double getSRflow() const;

protected:

    std::vector<double> control_line;

    std::vector<double> valve_state;

    double pBC;
    double p_airdistBC;
    double pSR;
    double p_airdistSR;

    double Q_airdistBC;
    double QBC;
    double Q_airdistSR;
    double QSR;
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
typedef ElectroAirDistributor* (*GetElectroAirDistributor)();

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
#define GET_ELECTRO_AIRDISTRIBUTOR(ClassName) \
    extern "C" Q_DECL_EXPORT ElectroAirDistributor *getElectroAirDistributor() \
    { \
        return new (ClassName) (); \
    }

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
extern "C" Q_DECL_EXPORT ElectroAirDistributor *loadElectroAirDistributor(QString lib_path);

#endif // ELECTRO_AIRDISTRIBUTOR_H
