#include    <bistate-handler.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool BiStateHandler::load_config(CfgReader &cfg, QDomNode secNode)
{
    ControlHandler::load_config(cfg, secNode);

    QString tmp = "";

    if (cfg.getString(secNode, "PositionsNames", tmp))
    {
        positionNames = tmp.isEmpty() ? QStringList() : tmp.split(',');
    }

    tmp = "";
    if (cfg.getString(secNode, "ActionNames", tmp))
    {
        actionNames = tmp.isEmpty() ? QStringList() : tmp.split(',');
    }

    return true;
}
