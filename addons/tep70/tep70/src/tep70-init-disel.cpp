#include    "tep70.h"

#include    <QDir>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TEP70::initDisel()
{
    disel = new Disel();
    disel->read_custom_config(config_dir + QDir::separator() + "disel");
}
