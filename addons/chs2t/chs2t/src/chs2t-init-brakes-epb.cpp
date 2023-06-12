#include    "filesystem.h"

#include    "chs2t.h"

#include    "Journal.h"

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
void CHS2T::initEPB(QString modules_dir)
{
    Q_UNUSED(modules_dir)

    //Journal::instance()->info("Init electropneumatic brakes");

    // Преобразователь напряжения для ЭПТ
    epb_converter = new EPBConverter();
    epb_converter->read_config("epb-converter");

    // Контроллер двухпроводного ЭПТ
    epb_control = new EPBControl();
}
