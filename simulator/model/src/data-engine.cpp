#include    "data-engine.h"
#include    "data-prepare.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
DataEngine::DataEngine() : AbstractEngineDefiner ()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
AbstractDataEngine *DataEngine::getDataEngine_(QString name)
{
    Q_UNUSED(name)
    return new DataPrepare();
}
