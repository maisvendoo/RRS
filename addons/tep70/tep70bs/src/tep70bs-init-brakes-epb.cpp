#include    "filesystem.h"

#include    "tep70bs.h"

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
void TEP70BS::initEPB(QString modules_dir)
{
    Q_UNUSED(modules_dir)

    // Преобразователь напряжения для ЭПТ
    epb_converter = new EPBConverter();
    epb_converter->read_config("epb-converter-110V");

    // Контроллер двухпроводного ЭПТ
    epb_control = new EPBControl();
}
