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

    void setEndCoord(double x_end) { this->x_end = x_end; }

    double getLenght() const { return qAbs(x_end - x_begin); }

    void setPrevSection(BlockSection *section) { prev_section = section; }

    void setNextSection(BlockSection *section) { next_section = section; }

    BlockSection *getNextSection() const { return next_section; }

    BlockSection *getPrevSection() const { return prev_section; }

    void setSignal(Signal *signal) { this->signal = signal; }

protected:

    double  x_begin;

    double  x_end;

    bool    is_busy;

    BlockSection *prev_section;

    BlockSection *next_section;

    Signal  *signal;
};

#endif // BLOCK_SECTION_H
