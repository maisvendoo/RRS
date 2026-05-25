#ifndef     FIND_SETTINGS_H
#define     FIND_SETTINGS_H

#include    <CfgEditor.h>
#include    <QVariant>

QPair<QString, QVariant> findSetting(QString setting,
                                     FieldsDataList &fd_list,
                                     int &idx);

QPair<QString, QVariant> findSetting(QString setting, FieldsDataList &fd_list);

#endif
