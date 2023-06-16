#include    "filesystem.h"

#include    "tep70.h"

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
void TEP70::initEPB(QString modules_dir)
{
    Q_UNUSED(modules_dir)

    // Преобразователь напряжения для ЭПТ
    epb_converter = new EPBConverter();
    epb_converter->read_config("epb-converter-110V");

    // Контроллер двухпроводного ЭПТ
    epb_control = new EPBControl();
}
