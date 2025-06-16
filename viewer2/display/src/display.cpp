#include "display.h"

#include "display-types.h"

#include <QApplication>
#include <QLibrary>
#include <QString>
#include <QWidget>

#include <algorithm>
#include <cstddef>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
AbstractDisplay::AbstractDisplay(QWidget* parent, Qt::WindowFlags flags)
    : QWidget(parent, flags)
{
    std::fill(input_signals.begin(), input_signals.end(), 0.0);
    std::fill(output_signals.begin(), output_signals.end(), 0.0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AbstractDisplay::setInputSignal(std::size_t index, float value)
{
    if (index < input_signals.size())
    {
        input_signals[index] = value;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float AbstractDisplay::getOutputSignal(std::size_t index)
{
    if (index < output_signals.size())
    {
        return output_signals[index];
    }

    return 0.0f;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
AbstractDisplay* loadDisplay(QString lib_path)
{
    AbstractDisplay* display = nullptr;

    QLibrary lib(lib_path);
    if (lib.load())
    {
        GetDisplay getDisplay = reinterpret_cast<GetDisplay>(lib.resolve("getDisplay"));
        if (getDisplay)
        {
            QMetaObject::invokeMethod(qApp, [&]() {
                display = getDisplay();
            }, Qt::BlockingQueuedConnection);
        }
    }

    return display;
}
