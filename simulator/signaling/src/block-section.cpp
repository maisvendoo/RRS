#include    "block-section.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
BlockSection::BlockSection(QObject *parent) : QObject(parent)
  , x_begin(0.0)
  , x_end(0.0)
  , is_busy(false)
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
void BlockSection::step(double t, double dt)
{
    if (signal != Q_NULLPTR)
    {
        signal->close(is_busy);
        signal->step(t, dt);
    }

    transmiter->setState(signal->getRedState(),
                         signal->getGreenState(),
                         signal->getYellowState());
    transmiter->step(t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BlockSection::slotRecvAlsnCode(int alsn_code)
{
    this->alsn_code = alsn_code;    
}
