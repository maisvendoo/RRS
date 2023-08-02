#include    "mainwindow.h"
#include    "ui_mainwindow.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotOnAddLODButtonClick()
{
    LOD_data_t LOD_data;
    LOD_data.level = LOD_list.size() + 1;

    LOD_list.push_back(LOD_data);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotOnDeleteLODButtonClick()
{

}
