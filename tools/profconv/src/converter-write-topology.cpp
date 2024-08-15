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
    std::string path = compinePath(toNativeSeparators(trajectoriesDir), trajectory->name + FILE_EXTENTION);

    QFile file_old(QString(path.c_str()));
    if (file_old.exists())
        file_old.rename( QString((path + FILE_BACKUP_EXTENTION).c_str()) );

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
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ZDSimConverter::writeTopologyConnectors()
{
    std::string path = compinePath(toNativeSeparators(topologyDir), FILE_TOPOLOGY);

    QFile file_old(QString(path.c_str()));
    if (file_old.exists())
        file_old.rename( QString((path + FILE_BACKUP_EXTENTION).c_str()) );

    CfgEditor editor;
    editor.openFileForWrite(QString(path.c_str()));

    size_t i = 0;
    // Соединения главного пути "туда" и однопутных участков
    for (auto connector : split_data1)
    {
        ++i;
        FieldsDataList flist;
        flist.append(QPair<QString, QString>("Name", QString("%1").arg(i, 5, 10, QChar('0'))));
        // Основной путь, сзади и спереди
        flist.append(QPair<QString, QString>("bwdMinusTraj", QString(connector->bwd_main_traj.c_str())));
        flist.append(QPair<QString, QString>("fwdMinusTraj", QString(connector->fwd_main_traj.c_str())));

        // Стрелка на бок сзади
        QString bwd_side = QString(connector->bwd_side_traj.c_str());
        if (!bwd_side.isEmpty())
        {
            flist.append(QPair<QString, QString>("bwdPlusTraj", bwd_side));
            flist.append(QPair<QString, QString>("state", QString("-1"))); // РАЗДЕЛИТЬ "state" ДЛЯ СТРЕЛОК СПЕРЕДИ И СЗАДИ
        }
        // Стрелка на бок спереди
        QString fwd_side = QString(connector->bwd_side_traj.c_str());
        if (!fwd_side.isEmpty())
        {
            flist.append(QPair<QString, QString>("fwdPlusTraj", fwd_side));
            flist.append(QPair<QString, QString>("state", QString("-1"))); // РАЗДЕЛИТЬ "state" ДЛЯ СТРЕЛОК СПЕРЕДИ И СЗАДИ
        }

        // Светофор назад
        QString bwd_liter = QString(connector->signal_bwd_liter.c_str());
        if (!bwd_liter.isEmpty())
        {
            flist.append(QPair<QString, QString>("SignalLetter", bwd_liter)); // РАЗДЕЛИТЬ ДЛЯ СВЕТОФОРОВ ВПЕРЁД И НАЗАД
            QString bwd_type = QString(connector->signal_bwd_type.c_str());
            if (!bwd_type.isEmpty())
                flist.append(QPair<QString, QString>("SignalModel", bwd_type));
        }
        // Светофор вперёд
        QString fwd_liter = QString(connector->signal_fwd_liter.c_str());
        if (!fwd_liter.isEmpty())
        {
            flist.append(QPair<QString, QString>("SignalLetter", fwd_liter)); // РАЗДЕЛИТЬ ДЛЯ СВЕТОФОРОВ ВПЕРЁД И НАЗАД
            QString fwd_type = QString(connector->signal_fwd_type.c_str());
            if (!fwd_type.isEmpty())
                flist.append(QPair<QString, QString>("SignalModel", fwd_type));
        }
        editor.writeFile("Joint", flist);
    }

    editor.closeFileAfterWrite();
}
