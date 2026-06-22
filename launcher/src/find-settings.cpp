#include    "find-settings.h"

#include    <CfgEditor.h>

#include    <QString>
#include    <QVariant>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void changeSetting(const QString& setting, FieldsDataList& fd_list,
    const QVariant& new_value)
{
    for (auto& pair : fd_list)
    {
        if (pair.first == setting)
        {
            pair.second = new_value;
            return;
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
const QVariant& getSetting(const QString& setting,
    const FieldsDataList& fd_list)
{
    for (const auto& pair : fd_list)
    {
        if (pair.first == setting)
        {
            return pair.second;
        }
    }

    static QVariant variant;
    return variant;
}
