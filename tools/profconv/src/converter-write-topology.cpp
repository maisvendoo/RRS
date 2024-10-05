#include    "converter.h"

#include    <QFile>
#include    <QVariant>

#include    "path-utils.h"
#include    "CfgEditor.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ZDSimConverter::writeTopologyTrajectory(const trajectory_t* trajectory)
{
    std::string path = compinePath(trajectoriesDir, trajectory->name + FILE_TRAJ_EXTENTION);

    QFile file(QString(path.c_str()));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);
    stream.setRealNumberNotation(QTextStream::FixedNotation);

    for (auto point = trajectory->points.begin(); point != trajectory->points.end(); ++point)
    {
        stream                  << point->point.x
            << DELIMITER_SYMBOL << point->point.y
            << DELIMITER_SYMBOL << point->point.z
            << DELIMITER_SYMBOL << static_cast<int>(round(point->railway_coord))
            << DELIMITER_SYMBOL << point->trajectory_coord
            << "\n";
    }

    file.close();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void addTopologyNode(CfgEditor &editor, split_zds_trajectory_t* connector, size_t i)
{
    FieldsDataList flist;
    flist.append(QPair<QString, QString>("Name", QString("%1").arg(i, 5, 10, QChar('0'))));
    // Основной путь, сзади и спереди
    flist.append(QPair<QString, QString>("bwdPlusTraj", QString(connector->bwd_main_traj.c_str())));
    flist.append(QPair<QString, QString>("fwdPlusTraj", QString(connector->fwd_main_traj.c_str())));

    // Стрелка на бок сзади
    QString bwd_side = QString(connector->bwd_side_traj.c_str());
    if (!bwd_side.isEmpty())
    {
        flist.append(QPair<QString, QString>("bwdMinusTraj", bwd_side));
        flist.append(QPair<QString, QString>("state_bwd", QString("1")));
    }
    // Стрелка на бок спереди
    QString fwd_side = QString(connector->fwd_side_traj.c_str());
    if (!fwd_side.isEmpty())
    {
        flist.append(QPair<QString, QString>("fwdMinusTraj", fwd_side));
        flist.append(QPair<QString, QString>("state_fwd", QString("1")));
    }

    // Светофор назад
    QString bwd_liter = QString(connector->signal_bwd_liter.c_str());
    if (!bwd_liter.isEmpty())
    {
        flist.append(QPair<QString, QString>("SignalLiterBwd", bwd_liter));
        QString bwd_type = QString(connector->signal_bwd_type.c_str());

        if (!bwd_type.isEmpty())
        {
            flist.append(QPair<QString, QString>("SignalModelBwd", bwd_type));
            //flist.append(QPair<QString, QString>("SignalDirection", QString("-1")));
        }
    }
    // Светофор вперёд
    QString fwd_liter = QString(connector->signal_fwd_liter.c_str());
    if (!fwd_liter.isEmpty())
    {
        flist.append(QPair<QString, QString>("SignalLiterFwd", fwd_liter));
        QString fwd_type = QString(connector->signal_fwd_type.c_str());
        if (!fwd_type.isEmpty())
        {
            flist.append(QPair<QString, QString>("SignalModelFwd", fwd_type));
            //flist.append(QPair<QString, QString>("SignalDirection", QString("1")));
        }
    }
    //editor.writeFile("Joint", flist);
    editor.writeFile("Switch", flist);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ZDSimConverter::writeTopologyConnectors()
{
    std::string path = compinePath(topologyDir, FILE_TOPOLOGY);

    QFile file_old(QString(path.c_str()));
    if (file_old.exists())
    {
        std::string backup = FILE_BACKUP_PREFIX + FILE_TOPOLOGY + FILE_BACKUP_EXTENTION;
        file_old.rename( QString(compinePath(topologyDir, backup).c_str()) );
    }

    CfgEditor editor;
    editor.openFileForWrite(QString(path.c_str()));
    editor.setIndentationFormat(-1);

    size_t i = 0;
    // Соединения главного пути "туда" и однопутных участков
    for (auto connector : split_data1)
    {
        ++i;
        addTopologyNode(editor, connector, i);
    }
    // Соединения главного пути "обратно" и однопутных участков
    for (auto connector : split_data2)
    {
        ++i;
        addTopologyNode(editor, connector, i);
    }
    // Соединения боковых путей по светофорам
    for (auto connector : branch_connectors)
    {
        ++i;
        addTopologyNode(editor, connector, i);
    }

    editor.closeFileAfterWrite();
}
