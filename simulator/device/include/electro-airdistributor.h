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

    // Электромагнитные вентили в электровоздухораспределителе
    /// Задать количество электромагнитных вентилей
    void setValvesNumber(size_t num);

    /// Количество электромагнитных вентилей
    size_t getValvesNumber() const;

    /// Состояние электромагнитных вентилей
    bool getValveState(size_t idx);

    // Взаимодействие электровоздухораспределителя с линиями управления ЭПТ
    /// Задать число линий ЭПТ, подключенных к электровоздухораспределителю
    void setControlLinesNumber(size_t num);

    /// Число линий ЭПТ, подключенных к электровоздухораспределителю
    size_t getControlLinesNumber() const;

    /// Задать величину постоянного напряжения в линии idx
    void setVoltage(size_t idx, double value);

    /// Задать частоту переменного напряжения в линии idx
    void setFrequency(size_t idx, double value);

    /// Получить потребляемый ток из линии idx
    double getCurrent(size_t idx) const;

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

    size_t valves_num;
    std::vector<bool> valve_state;

    size_t lines_num;
    std::vector<double> U;
    std::vector<double> f;
    std::vector<double> I;

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
