#ifndef     DISPLAY_H
#define     DISPLAY_H

#include    <QWidget>

#include    "display-export.h"
#include    "display-types.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DISPLAY_EXPORT AbstractDisplay : public QWidget
{
public:

    AbstractDisplay(QWidget *parent = Q_NULLPTR,
                    Qt::WindowFlags f = Qt::WindowFlags());

    virtual ~AbstractDisplay();

    void setInputSignal(size_t index, float value);

    float getOutputSignal(size_t index);

    void setInputSignals(const display_signals_t &input_signals);

    display_signals_t getOutputSignals();

protected:

    /// Входные сигналы, отображаемые на интерфейсе дисплея и управляющие его поведением
    display_signals_t input_signals;

    /// Выходные (командные) сигнала, передаваемые с дисплея
    display_signals_t output_signals;
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
//typedef std::vector<AbstractDisplay *> displays_t;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
#define TO_BOOL(SignalName) static_cast<bool>(SignalName)

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
typedef AbstractDisplay* (*GetDisplay)();

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
#define GET_DISPLAY(ClassName) \
    extern "C" Q_DECL_EXPORT AbstractDisplay *getDisplay() \
    {\
        return new (ClassName)(); \
    }

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
extern "C" Q_DECL_EXPORT AbstractDisplay *loadDisplay(QString lib_path);

#endif // DISPLAY_H
