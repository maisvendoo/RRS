#include    <QLibrary>

#include    "pneumo-hose-epb.h"

#include    "CfgReader.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
PneumoHoseEPB::PneumoHoseEPB(QObject *parent) : PneumoHose(parent)
  , lines_num(1)
{
    name = QString("BPepb");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
PneumoHoseEPB::~PneumoHoseEPB()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoHoseEPB::setLinesNumber(size_t num)
{
    lines_num = num;

    size_t old_input_size = input_signals.size();
    size_t old_output_size = output_signals.size();
    size_t new_signals_size = BEGIN_OF_EPB_LINE_SIGNALS
           + lines_num * SIZE_OF_SIGNALS_PER_EPB_LINE;

    input_signals.resize(new_signals_size);
    output_signals.resize(new_signals_size);

    if (old_input_size < new_signals_size)
        std::fill(  input_signals.begin() + old_input_size
                  , input_signals.end()
                  , 0.0);

    if (old_output_size < new_signals_size)
        std::fill(  output_signals.begin() + old_output_size
                  , output_signals.end()
                  , 0.0);

    output_signals[HOSE_OUTPUT_EPB_LINES_NUM] = static_cast<double>(lines_num);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
size_t PneumoHoseEPB::getLinesNumber() const
{
    return lines_num;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
size_t PneumoHoseEPB::getConnectedLinesNumber() const
{
    return static_cast<size_t>(input_signals[HOSE_INPUT_EPB_CONNECTED_NUM]);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoHoseEPB::setVoltage(size_t idx, double value)
{
    size_t i = BEGIN_OF_EPB_LINE_SIGNALS
             + idx * SIZE_OF_SIGNALS_PER_EPB_LINE
             + EPB_LINE_OUTPUT_VOLTAGE;

    if (i < output_signals.size())
        output_signals[i] = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoHoseEPB::setFrequency(size_t idx, double value)
{
    size_t i = BEGIN_OF_EPB_LINE_SIGNALS
             + idx * SIZE_OF_SIGNALS_PER_EPB_LINE
             + EPB_LINE_OUTPUT_FREQUENCY;

    if (i < output_signals.size())
        output_signals[i] = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoHoseEPB::setCurrent(size_t idx, double value)
{
    size_t i = BEGIN_OF_EPB_LINE_SIGNALS
             + idx * SIZE_OF_SIGNALS_PER_EPB_LINE
             + EPB_LINE_OUTPUT_CURRENT;

    if (i < output_signals.size())
        output_signals[i] = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double PneumoHoseEPB::getVoltage(size_t idx)
{
    size_t i = BEGIN_OF_EPB_LINE_SIGNALS
             + idx * SIZE_OF_SIGNALS_PER_EPB_LINE
             + EPB_LINE_INPUT_VOLTAGE;

    if (i < input_signals.size())
        return input_signals[i];
    else
        return 0.0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double PneumoHoseEPB::getFrequency(size_t idx)
{
    size_t i = BEGIN_OF_EPB_LINE_SIGNALS
             + idx * SIZE_OF_SIGNALS_PER_EPB_LINE
             + EPB_LINE_INPUT_FREQUENCY;

    if (i < input_signals.size())
        return input_signals[i];
    else
        return 0.0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double PneumoHoseEPB::getCurrent(size_t idx)
{
    size_t i = BEGIN_OF_EPB_LINE_SIGNALS
             + idx * SIZE_OF_SIGNALS_PER_EPB_LINE
             + EPB_LINE_INPUT_CURRENT;

    if (i < input_signals.size())
        return input_signals[i];
    else
        return 0.0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoHoseEPB::step(double t, double dt)
{
    PneumoHose::step(t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoHoseEPB::load_config(CfgReader &cfg)
{
    QString secName = "Device";

    cfg.getString(secName, "Name", name);

    double tmp = 1.0;
    cfg.getDouble(secName, "FlowCoefficient", tmp);
    output_signals[HOSE_OUTPUT_FLOW_COEFF] = tmp;

    int num = 0;
    cfg.getInt(secName, "LinesNum", num);
    if (num > 0)
        setLinesNumber(static_cast<size_t>(num));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
PneumoHoseEPB *loadPneumoHoseEPB(QString lib_path)
{
    PneumoHoseEPB *hose_epb = nullptr;

    QLibrary lib(lib_path);

    if (lib.load())
    {
        GetPneumoHoseEPB getPneumoHoseEPB = reinterpret_cast<GetPneumoHoseEPB>(lib.resolve("getPneumoHoseEPB"));

        if (getPneumoHoseEPB)
        {
            hose_epb = getPneumoHoseEPB();
        }
    }

    return hose_epb;
}
