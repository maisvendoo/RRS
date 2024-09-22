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

    for (auto traj = trajectories1.begin(); traj != trajectories1.end(); ++traj)
    {
        if ((*traj)->ALSN_frequency == 25)
            flist_25hz.append(QPair<QString, QString>("Trajectory", QString((*traj)->name.c_str())));
        if ((*traj)->ALSN_frequency == 50)
            flist_50hz.append(QPair<QString, QString>("Trajectory", QString((*traj)->name.c_str())));
    }
    for (auto traj = trajectories2.begin(); traj != trajectories2.end(); ++traj)
    {
        if ((*traj)->ALSN_frequency == 25)
            flist_25hz.append(QPair<QString, QString>("Trajectory", QString((*traj)->name.c_str())));
        if ((*traj)->ALSN_frequency == 50)
            flist_50hz.append(QPair<QString, QString>("Trajectory", QString((*traj)->name.c_str())));
    }
    for (auto it = branch_track_data1.begin(); it != branch_track_data1.end(); ++it)
    {
        for (auto traj = (*it)->trajectories.begin(); traj != (*it)->trajectories.end(); ++traj)
        {
            if ((*traj)->ALSN_frequency == 25)
                flist_25hz.append(QPair<QString, QString>("Trajectory", QString((*traj)->name.c_str())));
            if ((*traj)->ALSN_frequency == 50)
                flist_50hz.append(QPair<QString, QString>("Trajectory", QString((*traj)->name.c_str())));
        }
    }
    for (auto it = branch_track_data2.begin(); it != branch_track_data2.end(); ++it)
    {
        for (auto traj = (*it)->trajectories.begin(); traj != (*it)->trajectories.end(); ++traj)
        {
            if ((*traj)->ALSN_frequency == 25)
                flist_25hz.append(QPair<QString, QString>("Trajectory", QString((*traj)->name.c_str())));
            if ((*traj)->ALSN_frequency == 50)
                flist_50hz.append(QPair<QString, QString>("Trajectory", QString((*traj)->name.c_str())));
        }
    }
    if (!flist_25hz.empty())
    {
        flist_25hz.append(QPair<QString, QString>("Frequency", QString("25")));

        std::string path_25hz = compinePath(ALSN_Dir, FILE_ALSN_25HZ);
        CfgEditor editor_25hz;
        editor_25hz.openFileForWrite(QString(path_25hz.c_str()));
        editor_25hz.setIndentationFormat(-1);
        editor_25hz.writeFile("ALSN", flist_25hz);
        editor_25hz.closeFileAfterWrite();
    }
    if (!flist_50hz.empty())
    {
        flist_50hz.append(QPair<QString, QString>("Frequency", QString("50")));

        std::string path_50hz = compinePath(ALSN_Dir, FILE_ALSN_50HZ);
        CfgEditor editor_50hz;
        editor_50hz.openFileForWrite(QString(path_50hz.c_str()));
        editor_50hz.setIndentationFormat(-1);
        editor_50hz.writeFile("ALSN", flist_50hz);
        editor_50hz.closeFileAfterWrite();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ZDSimConverter::writeALSN_OLD()
{
    std::string path = compinePath(toNativeSeparators(topologyDir), FILE_ALSN);

    QFile file_old(QString(path.c_str()));
    if (file_old.exists())
        file_old.rename( QString((path + FILE_BACKUP_EXTENTION).c_str()) );

    CfgEditor editor;
    editor.openFileForWrite(QString(path.c_str()));
    editor.setIndentationFormat(-1);

    FieldsDataList flist_25hz;
    FieldsDataList flist_50hz;
    for (auto traj = trajectories1.begin(); traj != trajectories1.end(); ++traj)
    {
        if ((*traj)->ALSN_frequency == 25)
            flist_25hz.append(QPair<QString, QString>("Trajectory", QString((*traj)->name.c_str())));
        if ((*traj)->ALSN_frequency == 50)
            flist_50hz.append(QPair<QString, QString>("Trajectory", QString((*traj)->name.c_str())));
    }
    for (auto traj = trajectories2.begin(); traj != trajectories2.end(); ++traj)
    {
        if ((*traj)->ALSN_frequency == 25)
            flist_25hz.append(QPair<QString, QString>("Trajectory", QString((*traj)->name.c_str())));
        if ((*traj)->ALSN_frequency == 50)
            flist_50hz.append(QPair<QString, QString>("Trajectory", QString((*traj)->name.c_str())));
    }
    for (auto it = branch_track_data1.begin(); it != branch_track_data1.end(); ++it)
    {
        for (auto traj = (*it)->trajectories.begin(); traj != (*it)->trajectories.end(); ++traj)
        {
            if ((*traj)->ALSN_frequency == 25)
                flist_25hz.append(QPair<QString, QString>("Trajectory", QString((*traj)->name.c_str())));
            if ((*traj)->ALSN_frequency == 50)
                flist_50hz.append(QPair<QString, QString>("Trajectory", QString((*traj)->name.c_str())));
        }
    }
    for (auto it = branch_track_data2.begin(); it != branch_track_data2.end(); ++it)
    {
        for (auto traj = (*it)->trajectories.begin(); traj != (*it)->trajectories.end(); ++traj)
        {
            if ((*traj)->ALSN_frequency == 25)
                flist_25hz.append(QPair<QString, QString>("Trajectory", QString((*traj)->name.c_str())));
            if ((*traj)->ALSN_frequency == 50)
                flist_50hz.append(QPair<QString, QString>("Trajectory", QString((*traj)->name.c_str())));
        }
    }
    if (!flist_25hz.empty())
    {
        flist_25hz.append(QPair<QString, QString>("Frequency", QString("25")));
        editor.writeFile("ALSN", flist_25hz);
    }
    if (!flist_50hz.empty())
    {
        flist_50hz.append(QPair<QString, QString>("Frequency", QString("50")));
        editor.writeFile("ALSN", flist_50hz);
    }
    editor.closeFileAfterWrite();
}
