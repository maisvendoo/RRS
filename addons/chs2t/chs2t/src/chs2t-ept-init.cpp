#include    "chs2t.h"

#include    <QDir>

void CHS2T::initEPT()
{
    EPTControlLine *line = new EPTControlLine();
    line->read_custom_config(config_dir + QDir::separator() + "ept-line");

    eptLine.push_back(line);
}
