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

    void setControlLine(double value, size_t idx = 0);

    void setInputSupplyReservoirFlow(double Qar_in);

    double getOutputSupplyReservoirFlow();

protected:

    /// Расход воздуха на пополнение ЗР от ВР
    double Qar_in;
    /// Расход воздуха в ЗР
    double Qar_out;

    /// Давление в ЗР
    double p_ar;

    /// Расход воздуха на наполнение ТЦ
    double Qbc_out;
    /// Расход воздуха от ВР на наполнение ТЦ
    double Qbc_in;

    /// Давление в ТЦ
    double pbc_out;
    /// Давление в ТЦ, передаваемое в модель ВР
    double pbc_in;

    std::vector<double> control_line;
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
