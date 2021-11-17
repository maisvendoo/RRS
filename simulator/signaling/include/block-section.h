#ifndef     BLOCK_SECTION_H
#define     BLOCK_SECTION_H

#include    <QObject>

#include    "abstract-signal.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class BlockSection : public QObject
{
    Q_OBJECT

public:

    BlockSection(QObject *parent = Q_NULLPTR);

    ~BlockSection();

    void setBeginCoord(double x_begin) { this->x_begin = x_begin; }

    double getBeginCoord() const { return x_begin; }

    double getEndCoord() const { return x_end; }

    void setEndCoord(double x_end) { this->x_end = x_end; }

    double getLenght() const { return qAbs(x_end - x_begin); }

    void setPrevSection(BlockSection *section) { prev_section = section; }

    void setNextSection(BlockSection *section) { next_section = section; }

    BlockSection *getNextSection() const { return next_section; }

    BlockSection *getPrevSection() const { return prev_section; }

    void setSignal(Signal *signal) { this->signal = signal; }

    Signal *getSignal() const { return signal; }

    void setBusy(bool is_busy) { this->is_busy = is_busy; }

    void step(double t, double dt);

protected:

    double  x_begin;

    double  x_end;

    /// Признак занятости блок-участка
    bool    is_busy;

    /// Код АЛСН (будем получать через слот от путевого трансмитера!!!)
    int     alsn_code;

    BlockSection *prev_section;

    BlockSection *next_section;

    Signal  *signal;
};

#endif // BLOCK_SECTION_H
