#ifndef     BISTATE_HANDLER_H
#define     BISTATE_HANDLER_H

#include    <control-handler.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class BiStateHandler : public ControlHandler
{
public:

    explicit BiStateHandler(QObject *parent = nullptr) : ControlHandler(parent)
    {

    }

    virtual bool load_config(CfgReader &cfg, QDomNode secNode) override;

protected:

    QStringList positionNames;

    QStringList actionNames;

    QString getPositionName(int pos) const
    {
        if (pos < positionNames.size())
        {
            return positionNames[pos];
        }

        return "";
    }

    QString getActionName(int action_idx) const
    {
        if (action_idx < actionNames.size())
        {
            return actionNames[action_idx];
        }

        return "";
    }
};

#endif
