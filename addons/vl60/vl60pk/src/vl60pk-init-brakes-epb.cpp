#include    "filesystem.h"

#include    "vl60pk.h"

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
void VL60pk::initEPB(QString modules_dir)
{
    Q_UNUSED(modules_dir)

    // Преобразователь напряжения для ЭПТ
    epb_converter = new EPBConverter();
    epb_converter->read_config("epb-converter");

    // Контроллер двухпроводного ЭПТ
    epb_control = new EPBControl();
}
