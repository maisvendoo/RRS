#include    "electro-airdistributor.h"

#include    <QLibrary>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ElectroAirDistributor::ElectroAirDistributor(QObject *parent) : BrakeDevice(parent)
  , Qar_in(0.0)
  , Qar_out(0.0)
  , p_ar(0.0)
  , Qbc_out(0.0)
  , Q2(0.0)
  , pbc_out(0.0)
  , pbc_in(0.0)

  , p1(0.0)
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

