#ifndef     BLOCK_SECTION_H
#define     BLOCK_SECTION_H

#include    <QObject>

#include    "abstract-signal.h"
#include    "alsn-transmiter.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class BlockSection : public QObject
{
    Q_OBJECT

public:

    BlockSection(QObject *parent = Q_NULLPTR);

    ~BlockSection();

    /// Задать начальную координату
    void setBeginCoord(double x_begin) { this->x_begin = x_begin; }

    /// Получить начальную координату
    double getBeginCoord() const { return x_begin; }

    /// Задать конечную координату
    void setEndCoord(double x_end) { this->x_end = x_end; }

    /// Получить конечную координату
    double getEndCoord() const { return x_end; }

    /// Получить длину блок-участка
    double getLenght() const { return qAbs(x_end - x_begin); }

    /// Задать указатель на предыдущий блок-участок
    void setPrevSection(BlockSection *section) { prev_section = section; }

    /// Задать указатель на следующий блок-участок
    void setNextSection(BlockSection *section) { next_section = section; }

    /// Получить указатель на следующий блок-участок
    BlockSection *getNextSection() const { return next_section; }

    /// Получить указатель на предыдущий блок-участок
    BlockSection *getPrevSection() const { return prev_section; }

    /// Задать указатель на сигнал
    void setSignal(Signal *signal) { this->signal = signal; }

    /// Получить указатель на сигнал
    Signal *getSignal() const { return signal; }

    /// Задать занятость блок-участка
    void setBusy(bool is_busy, double x_busy);

    /// Модулирование устройств СЦБ на блок-участке
    void step(double t, double dt);

    /// Получить указатель на путевой трансмитер
    Transmiter *getTransmiter() const { return transmiter; }

    /// Получить код АЛСН
    short getAlsnCode() const { return alsn_code; }

    /// Получить координату, на которой замкнута рельсовая цепь
    double getBusyCoord() const { return x_busy; }

protected:

    /// Координата начала блок-участка
    double  x_begin;

    /// Координата конца блок-участка
    double  x_end;

    /// Признак занятости блок-участка
    bool    is_busy;

    /// Число поездов, занявших блок-участок
    int     busy_count;

    /// Код АЛСН (будем получать через слот от путевого трансмитера!!!)
    short    alsn_code;

    /// Координата, на которой произведено замыкание рельсовой цепи
    double  x_busy;

    /// Указатель на предыдущий блок-участок
    BlockSection *prev_section;

    /// Указатель на следующий блок участок
    BlockSection *next_section;

    /// Сигнал (светофор) в начале блок участка
    Signal  *signal;

    /// Путевой трансмитер в начале блок-участка
    Transmiter *transmiter;

public slots:

    /// Прием кода от путевого трансмитера на предыдущем блок-участке
    void slotRecvAlsnCode(int alsn_code);
};

#endif // BLOCK_SECTION_H
