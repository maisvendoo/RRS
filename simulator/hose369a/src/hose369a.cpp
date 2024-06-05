#include    "hose369a.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Hose369A::Hose369A(int key_code, QObject *parent)
    : PneumoHoseEPB(key_code, parent)
    , is_hose_at_loco(false)
{
    name = QString("BPepb");
    setLinesNumber(EPB_LINES_NUM);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Hose369A::~Hose369A()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Hose369A::step(double t, double dt)
{
    // Стандартная работа рукава
    PneumoHoseEPB::step(t, dt);

    // Если линии ЭПТ соединены - больше ничего не делаем
    if (input_signals[HOSE_INPUT_EPB_CONNECTED_NUM] > 0.0)
    {
        return;
    }

    // Если рукав подвешен на локомотиве - больше ничего не делаем
    if (is_hose_at_loco && (input_signals[HOSE_INPUT_IS_CONNECTED] == 0.0))
    {
        return;
    }

    // В остальных случаях рабочая и контрольная линия ЭПТ
    // замыкаются в оголовке рукава
    size_t bias_work    = BEGIN_OF_EPB_LINE_SIGNALS
           + EPB_WORK_LINE * SIZE_OF_SIGNALS_PER_EPB_LINE;

    size_t bias_control = BEGIN_OF_EPB_LINE_SIGNALS
           + EPB_CONTROL_LINE * SIZE_OF_SIGNALS_PER_EPB_LINE;

    for (size_t i = 0; i < SIZE_OF_SIGNALS_PER_EPB_LINE; ++i)
    {
        input_signals[bias_work + i] = output_signals[bias_control + i];
        input_signals[bias_control + i] = output_signals[bias_work + i];
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Hose369A::load_config(CfgReader &cfg)
{
    QString secName = "Device";

    cfg.getString(secName, "Name", name);

    double tmp = 1.0;
    cfg.getDouble(secName, "FlowCoefficient", tmp);
    output_signals[HOSE_OUTPUT_FLOW_COEFF] = tmp;

    tmp = 0.71;
    cfg.getDouble(secName, "Length", tmp);
    if (tmp > Physics::ZERO)
        output_signals[HOSE_OUTPUT_LENGTH] = tmp;

    int num = 0;
    cfg.getInt(secName, "LinesNum", num);
    if (num > 0)
        setLinesNumber(static_cast<size_t>(num));

    cfg.getBool(secName, "IsHoseAtLoco", is_hose_at_loco);
}

GET_PNEUMO_HOSE_EPB(Hose369A)
