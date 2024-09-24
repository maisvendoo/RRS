#include    "converter.h"

#include    <QDir>
#include    <QVariant>

#include    "path-utils.h"
#include    "CfgEditor.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ZDSimConverter::writeSpeedmap()
{
    if (speedmap_data.empty())
        return;

    QString node = CONFIGNODE_TRAJ_3LVL.c_str();
    for (auto speedmap_element : speedmap_data)
    {
        if (speedmap_element->trajectories_names.empty() || speedmap_element->speedmap_elements.empty())
            continue;

        int coord_begin = speedmap_element->speedmap_elements.front().railway_coord_begin;
        int coord_end = speedmap_element->speedmap_elements.back().railway_coord_end;
        QString filename = speedmap_element->name_prefix.c_str();
        if (coord_begin >= 0)
            filename += QString("_%1_%2").arg(coord_begin).arg(coord_end);
        filename += ".xml";
        std::string path = compinePath(speedmapDir, filename.toStdString());

        CfgEditor editor;
        editor.openFileForWrite(QString(path.c_str()));
        editor.setIndentationFormat(-1);

        FieldsDataList flist;
        for (auto tn : speedmap_element->trajectories_names)
        {
            flist.append(QPair<QString, QString>(node, QString(tn.c_str())));
        }
        // Список траекторий
        QString node2 = CONFIGNODE_TRAJ_2LVL.c_str();
        editor.writeFile(node2, flist);

        FieldsDataList flist_speed;
        QString node_speedlimit = "SpeedLimit";
        node2 = "SpeedMap";
        for (auto se : speedmap_element->speedmap_elements)
        {
            QString field_value = QString("%1 %2 %3")
                                      .arg(se.limit)
                                      .arg(se.railway_coord_begin)
                                      .arg(se.railway_coord_end);
            flist_speed.append(QPair<QString, QString>(node_speedlimit, field_value));
        }
        editor.writeFile(node2, flist_speed);
        editor.closeFileAfterWrite();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ZDSimConverter::writeSpeedmap_OLD()
{
    std::string path = compinePath(toNativeSeparators(topologyDir), FILE_SPEEDMAP);

    QFile file_old(QString(path.c_str()));
    if (file_old.exists())
        file_old.rename( QString((path + FILE_BACKUP_EXTENTION).c_str()) );

    if (speedmap_data.empty())
        return;

    CfgEditor editor;
    editor.openFileForWrite(QString(path.c_str()));
    editor.setIndentationFormat(-1);

    for (auto speedmap_element : speedmap_data)
    {
        FieldsDataList flist;
        for (auto tn : speedmap_element->trajectories_names)
        {
            flist.append(QPair<QString, QString>("Trajectory", QString(tn.c_str())));
        }
        for (auto se : speedmap_element->speedmap_elements)
        {
            QString field_value = QString("%1 %2 %3")
                                      .arg(se.limit)
                                      .arg(se.railway_coord_begin)
                                      .arg(se.railway_coord_end);
            flist.append(QPair<QString, QString>("SpeedLimit", field_value));
        }
        editor.writeFile("SpeedMap", flist);
    }
    editor.closeFileAfterWrite();
}
