#include    "electro-airdistributor.h"

#include    <QLibrary>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ElectroAirDistributor::ElectroAirDistributor(QObject *parent) : BrakeDevice(parent)
  , pBC(0.0)
  , p_airdistBC(0.0)
  , pSR(0.0)
  , p_airdistSR(0.0)
  , Q_airdistBC(0.0)
  , QBC(0.0)
  , Q_airdistSR(0.0)
  , QSR(0.0)
{
    setControlLinesNumber(1);
    setValvesNumber(2);
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

void ElectroAirDistributor::setControlLinesNumber(size_t num)
{
    control_line.resize(num);
    std::fill(control_line.begin(), control_line.end(), 0.0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ElectroAirDistributor::setValvesNumber(size_t num)
{
    valve_state.resize(num);
    std::fill(valve_state.begin(), valve_state.end(), 0.0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ElectroAirDistributor::setControlLine(double value, size_t idx)
{
    if (idx < control_line.size())
        control_line[idx] = cut(value, -1.0, 1.0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double ElectroAirDistributor::getValveState(size_t i)
{
    if (i < valve_state.size())
        return valve_state[i];
    else
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

