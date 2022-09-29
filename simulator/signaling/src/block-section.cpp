#include    "block-section.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
BlockSection::BlockSection(QObject *parent) : QObject(parent)
  , x_begin(0.0)
  , x_end(0.0)
  , is_busy(false)
  , busy_count(0)
  , alsn_code(0)
  , x_busy(0.0)
  , prev_section(Q_NULLPTR)
  , next_section(Q_NULLPTR)
  , signal(Q_NULLPTR)
  , transmiter(new Transmiter)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
BlockSection::~BlockSection()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BlockSection::setBusy(bool is_busy, double x_busy)
{
    if (!this->is_busy)
        this->x_busy = x_busy;

    this->is_busy = is_busy;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BlockSection::step(double t, double dt)
{
    if (signal != Q_NULLPTR)
    {
        signal->close(is_busy);
        signal->step(t, dt);


        transmiter->setState(signal->getRedState(),
                             signal->getGreenState(),
                             signal->getYellowState());
    }

    transmiter->step(t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BlockSection::slotRecvAlsnCode(int alsn_code)
{
    this->alsn_code = alsn_code;
}
