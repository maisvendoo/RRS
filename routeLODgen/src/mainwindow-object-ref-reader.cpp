#include    "mainwindow.h"
#include    "ui_mainwindow.h"

#include    <QDir>
#include    <QFileDialog>
#include    <QFile>
#include    <QTextStream>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::objectRefReader()
{
    routeDir = QFileDialog::getExistingDirectory(this, tr("Open route"),
                                                 routesRootDir,
                                                 QFileDialog::ShowDirsOnly |
                                                 QFileDialog::DontResolveSymlinks);

    routeDir = QDir::toNativeSeparators(routeDir);

    QDir dir(routesRootDir);
    QString relPath = dir.relativeFilePath(routeDir);

    QString objects_ref_path = routeDir + QDir::separator() + "objects.ref";

    QFile objects_ref_file(objects_ref_path);

    if (!objects_ref_file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        ui->statusbar->showMessage(tr("ERROR: objects.ref file is't found"));
        return;
    }

    QTextStream ss(&objects_ref_file);

    QString content = ss.readAll();
    content = content.remove('\r');

    QStringList lines = content.split('\n');


    foreach (QString line, lines)
    {
        if ( (line[0] == ';') or (line[0] == '[') )
            continue;

        QStringList tokens = line.split('\t');

        if (tokens.size() < 3)
            continue;

        object_data_t object_data;
        object_data.model_ID = tokens[0];
        object_data.model_path = routeDir + QDir::toNativeSeparators(tokens[1]);

        for (int i = 2; i < tokens.count(); ++i)
        {
            object_data.texture_path.push_back(routeDir + QDir::toNativeSeparators(tokens[i]));
        }

        objects_ref.insert(object_data.model_ID, object_data);
    }

    findUsedModels();

    ui->labelCurrentRoute->setText(relPath);
    ui->labelAllObjectsNum->setText(QString("%1").arg(objects_ref.count()));
    ui->labelUsedObjectsNum->setText(QString("%1").arg(used_models.count()));
    ui->statusbar->showMessage(tr("Route loaded succesfully"));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::findUsedModels()
{
    QString map_path = routeDir + QDir::separator() + "route1.map";

    QFile map_file(map_path);

    if (!map_file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        ui->statusbar->showMessage(tr("ERROR: route1.map file is't found"));
        return;
    }

    QTextStream ss(&map_file);

    QString content = ss.readAll();

    content = content.remove('\r');

    QStringList lines = content.split('\n');

    QString model_ID = "";

    foreach (QString line, lines)
    {
        if (line.isEmpty())
            continue;

        if (line[0] == ';')
            continue;

        if (line[0] == ',')
            line = model_ID + line;

        QStringList token = line.split(',');

        model_ID = token[0];

        if (objects_ref.contains(model_ID))
        {
            used_models.insert(model_ID, objects_ref[model_ID].model_path);
        }
        else
        {
            unused_models.insert(model_ID, objects_ref[model_ID]);
        }
    }
}
