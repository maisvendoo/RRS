//------------------------------------------------------------------------------
//
//
//
//
//------------------------------------------------------------------------------
#ifndef DISPLAY_H
#define DISPLAY_H

#include "display-export.h"
#include "display-types.h"

#include <QString>
#include <QWidget>

#include <cstddef>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DISPLAY_EXPORT AbstractDisplay : public QWidget
{
public:
    AbstractDisplay(QWidget* parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());

    virtual ~AbstractDisplay() = default;

    /// Инициализация дисплея
    virtual void init() = 0;

    /// Обновление дисплея
    virtual void update(double t, double dt) = 0;

    /// Задать входной сигнал
    void setInputSignal(std::size_t index, float value);

    /// Задать массив входных сигналов
    void setInputSignals(const display_signals_t& input_signals) { this->input_signals = input_signals; }

    /// Получить выходной сигнал
    float getOutputSignal(std::size_t index);

    /// Получить массив выходных сигналов
    display_signals_t getOutputSignals() { return output_signals; }

    /// Задать каталог с конфигурационными файлами
    void setConfigDir(QString config_dir) { this->config_dir = config_dir; }

    /// Получить путь к каталогу с конфигами
    QString getConfigDir() const { return this->config_dir; }

    /// Задать путь к каталогу с маршрутом
    void setRouteDir(QString route_dir) { this->route_dir = route_dir; }

    void setUpdateInterval(double upd_interval) { this->upd_interval = upd_interval; }

protected:
    /// Входные сигналы, отображаемые на интерфейсе дисплея и управляющие его поведением
    display_signals_t input_signals = display_signals_t();

    /// Выходные (командные) сигналы, передаваемые с дисплея
    display_signals_t output_signals = display_signals_t();

    /// Путь к каталогу конфигурации
    QString config_dir = "";

    /// Путь к каталогу с текущим маршрутом
    QString route_dir = "";

    /// Интервал обновления
    double upd_interval = 0.5;
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
#define TO_BOOL(SignalName) static_cast<bool>(SignalName)

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
using GetDisplay = AbstractDisplay*(*)();

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
#define GET_DISPLAY(ClassName) \
    extern "C" DISPLAY_EXPORT AbstractDisplay* getDisplay() \
    { \
        return new (ClassName)(); \
    }

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
extern "C" DISPLAY_EXPORT AbstractDisplay* loadDisplay(QString lib_path);

#endif // DISPLAY_H
