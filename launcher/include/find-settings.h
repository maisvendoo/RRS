#ifndef     FIND_SETTINGS_H
#define     FIND_SETTINGS_H

#include    <CfgEditor.h>

#include    <QString>
#include    <QVariant>

void changeSetting(const QString& setting, FieldsDataList& fd_list,
    const QVariant& new_value);

const QVariant& getSetting(const QString& setting,
    const FieldsDataList& fd_list);

#endif // FIND_SETTINGS_H
