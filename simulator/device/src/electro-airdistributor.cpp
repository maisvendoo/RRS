#include    "electro-airdistributor.h"

#include    <QLibrary>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ElectroAirDistributor::ElectroAirDistributor(QObject *parent) : BrakeDevice(parent)
  , valves_num(0)
  , lines_num(0)
  , pBC(0.0)
  , p_airdistBC(0.0)
  , pSR(0.0)
  , p_airdistSR(0.0)
  , Q_airdistBC(0.0)
  , QBC(0.0)
  , Q_airdistSR(0.0)
  , QSR(0.0)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ElectroAirDistributor::~ElectroAirDistributor()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ElectroAirDistributor::setValvesNumber(size_t num)
{
    valves_num = num;
    valve_state.resize(valves_num);
    std::fill(valve_state.begin(), valve_state.end(), false);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
size_t ElectroAirDistributor::getValvesNumber() const
{
    return valves_num;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ElectroAirDistributor::getValveState(size_t idx)
{
    if (idx < valve_state.size())
        return valve_state[idx];
    return false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ElectroAirDistributor::setControlLinesNumber(size_t num)
{
    lines_num = num;

    U.resize(lines_num);
    f.resize(lines_num);
    I.resize(lines_num);

    std::fill(U.begin(), U.end(), 0.0);
    std::fill(f.begin(), f.end(), 0.0);
    std::fill(I.begin(), I.end(), 0.0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
size_t ElectroAirDistributor::getControlLinesNumber() const
{
    return lines_num;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ElectroAirDistributor::setVoltage(size_t idx, double value)
{
    if (idx < U.size())
        U[idx] = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ElectroAirDistributor::setFrequency(size_t idx, double value)
{
    if (idx < f.size())
        f[idx] = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double ElectroAirDistributor::getCurrent(size_t idx) const
{
    if (idx < I.size())
        return I[idx];
    return 0.0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ElectroAirDistributor::setBCpressure(double value)
{
    pBC = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double ElectroAirDistributor::getAirdistBCpressure() const
{
    return p_airdistBC;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ElectroAirDistributor::setAirdistBCflow(double value)
{
    Q_airdistBC = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double ElectroAirDistributor::getBCflow() const
{
    return QBC;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ElectroAirDistributor::setSRpressure(double value)
{
    pSR = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double ElectroAirDistributor::getAirdistSRpressure() const
{
    return p_airdistSR;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ElectroAirDistributor::setAirdistSRflow(double value)
{
    Q_airdistSR = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double ElectroAirDistributor::getSRflow() const
{
    return QSR;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ElectroAirDistributor *loadElectroAirDistributor(QString lib_path)
{

        ElectroAirDistributor *electro_airdist = nullptr;

        QLibrary lib(lib_path);

        if (lib.load())
        {
            GetElectroAirDistributor getElectroAirDistributor = reinterpret_cast<GetElectroAirDistributor>(lib.resolve("getElectroAirDistributor"));

            if (getElectroAirDistributor)
            {
                electro_airdist = getElectroAirDistributor();
            }
        }

        return electro_airdist;
}

