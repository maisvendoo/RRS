#include    "mainwindow.h"
#include    "ui_mainwindow.h"

#include    <QDir>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotOnCleanRoute()
{
    clean_count = 0;
    cleanTimer.start(100);

    ui->statusbar->showMessage(tr("Clean route in progress.."));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotOnCleanTimer()
{
    if (clean_count >= unused_models.size())
    {
        cleanTimer.stop();
        ui->statusbar->showMessage(tr("Ready"));
        return;
    }

    object_data_t object_data = unused_models[clean_count];

    QDir dir;

    dir.remove(object_data.model_path);
    ui->statusbar->showMessage(tr("Removed file") + object_data.model_path);

    for (auto it = object_data.texture_path.begin();
         it != object_data.texture_path.end();
         ++it)
    {
        dir.remove(*it);
        ui->statusbar->showMessage(tr("Removed file") + *it);
    }

    clean_count++;

    ui->progressBar->setValue(clean_count * 100 / unused_models.size());
}
