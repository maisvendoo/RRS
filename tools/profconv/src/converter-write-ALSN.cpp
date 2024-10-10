#include    "converter.h"

#include    <QFile>
#include    <QVariant>

#include    "path-utils.h"
#include    "CfgEditor.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ZDSimConverter::writeALSN()
{
    FieldsDataList flist_25hz;
    FieldsDataList flist_50hz;


    QStringList traj_list_25hz;
    QStringList traj_list_50hz;
    for (auto traj = trajectories1.begin(); traj != trajectories1.end(); ++traj)
    {
        if ((*traj)->ALSN_frequency == 25)
            traj_list_25hz.push_back((*traj)->name.c_str());
        if ((*traj)->ALSN_frequency == 50)
            traj_list_50hz.push_back((*traj)->name.c_str());
    }
    for (auto traj = trajectories2.begin(); traj != trajectories2.end(); ++traj)
    {
        if ((*traj)->ALSN_frequency == 25)
            traj_list_25hz.push_back((*traj)->name.c_str());
        if ((*traj)->ALSN_frequency == 50)
            traj_list_50hz.push_back((*traj)->name.c_str());
    }
    for (auto it = branch_track_data1.begin(); it != branch_track_data1.end(); ++it)
    {
        for (auto traj = (*it)->trajectories.begin(); traj != (*it)->trajectories.end(); ++traj)
        {
            if ((*traj)->ALSN_frequency == 25)
                traj_list_25hz.push_back((*traj)->name.c_str());
            if ((*traj)->ALSN_frequency == 50)
                traj_list_50hz.push_back((*traj)->name.c_str());
        }
    }
    for (auto it = branch_track_data2.begin(); it != branch_track_data2.end(); ++it)
    {
        for (auto traj = (*it)->trajectories.begin(); traj != (*it)->trajectories.end(); ++traj)
        {
            if ((*traj)->ALSN_frequency == 25)
                traj_list_25hz.push_back((*traj)->name.c_str());
            if ((*traj)->ALSN_frequency == 50)
                traj_list_50hz.push_back((*traj)->name.c_str());
        }
    }

    if (!traj_list_25hz.empty())
    {
        std::string path_25hz = compinePath(ALSN_Dir, FILE_ALSN_25HZ);
        CfgEditor editor_25hz;
        editor_25hz.openFileForWrite(QString(path_25hz.c_str()));
        editor_25hz.setIndentationFormat(-1);

        QString node = CONFIGNODE_TRAJ_3LVL.c_str();
        QString node2 = CONFIGNODE_TRAJ_2LVL.c_str();
        QString node_ALSN = "Frequency";
        QString node2_ALSN = "ALSN";

        // Список траекторий
        for (auto tn : traj_list_25hz)
        {
            FieldsDataList fdl = { QPair<QString, QString>(node, tn) };

            editor_25hz.writeFile(node2, fdl);
        }

        // Конфиг несущей частоты сигнала АЛСН
        FieldsDataList fdl = { QPair<QString, QString>(node_ALSN, QString("25")) };
        editor_25hz.writeFile(node2_ALSN, fdl);

        editor_25hz.closeFileAfterWrite();
    }

    if (!traj_list_50hz.empty())
    {
        std::string path_50hz = compinePath(ALSN_Dir, FILE_ALSN_50HZ);
        CfgEditor editor_50hz;
        editor_50hz.openFileForWrite(QString(path_50hz.c_str()));
        editor_50hz.setIndentationFormat(-1);

        QString node = CONFIGNODE_TRAJ_3LVL.c_str();
        QString node2 = CONFIGNODE_TRAJ_2LVL.c_str();
        QString node_ALSN = "Frequency";
        QString node2_ALSN = "ALSN";

        // Список траекторий
        for (auto tn : traj_list_50hz)
        {
            FieldsDataList fdl = { QPair<QString, QString>(node, tn) };

            editor_50hz.writeFile(node2, fdl);
        }

        // Конфиг несущей частоты сигнала АЛСН
        FieldsDataList fdl = { QPair<QString, QString>(node_ALSN, QString("50")) };
        editor_50hz.writeFile(node2_ALSN, fdl);

        editor_50hz.closeFileAfterWrite();
    }
}
