#include    "mainwindow.h"
#include    "ui_mainwindow.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotOnAddLODButtonClick()
{
    LOD_data_t LOD_data;
    LOD_data.level = LOD_list.size() + 1;

    if (LOD_list.size() > 0)
    {
        LOD_data.max_dist = (*(LOD_list.end() - 1)).min_dist;
    }

    LOD_list.push_back(LOD_data);

    int idx = ui->twLODparams->rowCount();
    ui->twLODparams->insertRow(idx);

    ui->twLODparams->setItem(idx, 0,
                             new QTableWidgetItem(QString("L%1").arg(LOD_data.level)));

    ui->twLODparams->setItem(idx, 1,
                             new QTableWidgetItem(QString("%1").arg(LOD_data.reduction, 4, '2', 2)));

    ui->twLODparams->setItem(idx, 2,
                             new QTableWidgetItem(QString("%1").arg(LOD_data.min_dist, 10, '2', 0)));

    ui->twLODparams->setItem(idx, 3,
                             new QTableWidgetItem(QString("%1").arg(LOD_data.max_dist, 10, '2', 0)));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotOnDeleteLODButtonClick()
{
    if (LOD_list.size() == 0)
        return;

    LOD_list.erase(LOD_list.end() - 1);

    int idx = ui->twLODparams->rowCount() - 1;
    ui->twLODparams->removeRow(idx);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotLODCellChanged(int row, int column)
{
    bool is_ok = false;

    switch (column)
    {
        case 1:
        {
            double reduction = ui->twLODparams->item(row, column)->text().toDouble(&is_ok);

            if (is_ok)
            {
                LOD_list[row].reduction = reduction;
            }

            break;
        }

        case 2:
        {
            double min_dist = ui->twLODparams->item(row, column)->text().toDouble(&is_ok);

            if (is_ok)
            {
                LOD_list[row].min_dist = min_dist;
            }

            break;
        }

        case 3:
        {
            double max_dist = ui->twLODparams->item(row, column)->text().toDouble(&is_ok);

            if (is_ok)
            {
                LOD_list[row].max_dist = max_dist;
            }
            break;
        }
    }
}
