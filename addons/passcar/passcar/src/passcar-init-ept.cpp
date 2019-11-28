#include    "passcar.h"

#include    <QDir>

void PassCarrige::initEPT()
{
    ept_control.resize(1);
    ept_control[0] = 0;

    ept_current.resize(1);
    ept_current[0] = 0;
}
