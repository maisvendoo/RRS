#include    "chs2t.h"

#include    <QDir>

void CHS2T::initEPT()
{
    ept_control.resize(1);
    ept_control[0] = 0;

    ept_current.resize(1);
    ept_current[0] = 0;

    ept_converter = new EPTConverter();
    ept_converter->read_config("ept-converter");

    ept_pass_control = new EPTPassControl();
}
