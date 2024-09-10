#include    "converter.h"

#include    <QFile>
#include    <QVariant>

#include    "path-utils.h"
#include    "CfgEditor.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ZDSimConverter::writeSpeedmap()
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
