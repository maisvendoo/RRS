#include    "display.h"

#include    <QLibrary>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
AbstractDisplay::AbstractDisplay(QWidget *parent, Qt::WindowFlags f)
    : QWidget(parent, f)

{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
AbstractDisplay::~AbstractDisplay()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AbstractDisplay::setInputSignal(size_t index, float value)
{
    if (index < input_signals.size())
        input_signals[index] = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float AbstractDisplay::getOutputSignal(size_t index)
{
    if (index < output_signals.size())
        return output_signals[index];

    return 0.0f;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AbstractDisplay::setInputSignals(const display_signals_t &input_signals)
{
    this->input_signals = input_signals;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
display_signals_t AbstractDisplay::getOutputSignals()
{
    return output_signals;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AbstractDisplay::setConfigDir(QString config_dir)
{
    this->config_dir = config_dir;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString AbstractDisplay::getConfigDir() const
{
    return this->config_dir;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AbstractDisplay::init()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
AbstractDisplay *loadDisplay(QString lib_path)
{
    AbstractDisplay *display = Q_NULLPTR;

    QLibrary lib(lib_path);

    if (lib.load())
    {
        GetDisplay getDisplay = reinterpret_cast<GetDisplay>(lib.resolve("getDisplay"));

        if (getDisplay != Q_NULLPTR)
        {
            display = getDisplay();
        }
    }

    return display;
}
