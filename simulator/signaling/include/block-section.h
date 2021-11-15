#ifndef     BLOCK_SECTION_H
#define     BLOCK_SECTION_H

#include    <QObject>

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

protected:

    double  x_begin;

    double  x_end;

    bool    is_busy;

    BlockSection *prev_section;

    BlockSection *next_section;
};

#endif // BLOCK_SECTION_H
