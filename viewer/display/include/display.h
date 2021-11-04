//------------------------------------------------------------------------------
//
//
//
//
//------------------------------------------------------------------------------
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

    /// Инициализация дисплея
    virtual void init();

    /// Задать входной сигнал
    void setInputSignal(size_t index, float value);

    /// Задать массив входных сигналов
    void setInputSignals(const display_signals_t &input_signals);

    /// Получить выходной сигнал
    float getOutputSignal(size_t index);

    /// Получить массив выходных сигналов
    display_signals_t getOutputSignals();

    /// Задать каталог с конфигруционными файлами
    void setConfigDir(QString config_dir);

    /// Получить путь к каталогу с конфигами
    QString getConfigDir() const;

    /// Задать путь к каталогу с маршрутом
    void setRouteDir(QString route_dir) { this->route_dir = route_dir; }

protected:

    /// Входные сигналы, отображаемые на интерфейсе дисплея и управляющие его поведением
    display_signals_t   input_signals;

    /// Выходные (командные) сигналы, передаваемые с дисплея
    display_signals_t   output_signals;

    /// Путь к каталогу конфигурации
    QString             config_dir;

    /// Путь к каталогу с текущим маршрутом
    QString             route_dir;
};

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
