#ifndef     MAINWINDOW_H
#define     MAINWINDOW_H

#include    <QMainWindow>
#include    <QMap>
#include    <QTimer>

#include    "object-data.h"
#include    "LOD-data.h"

QT_BEGIN_NAMESPACE
    namespace Ui { class MainWindow; }
QT_END_NAMESPACE

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    MainWindow(QWidget *parent = nullptr);

    ~MainWindow();

private:

    Ui::MainWindow *ui;

    QString routesRootDir;

    QString routeDir;

    size_t clean_count;

    int LOD_count;

    QTimer cleanTimer;

    QTimer LODGenTimer;

    QMap<QString, object_data_t> objects_ref;

    QMap<QString, QString> used_models;

    QMap<QString, QString>::iterator model_it;

    std::vector<object_data_t> unused_models;

    std::vector<LOD_data_t> LOD_list;

    void objectRefReader();

    void findUsedModels();

private slots:


    void slotOnQuit();

    void slotOnRouteOpen();

    void slotOnCleanRoute();

    void slotOnCleanTimer();

    void slotOnAddLODButtonClick();

    void slotOnDeleteLODButtonClick();

    void slotLODCellChanged(int row, int column);

    void slotOnLODGenerate();

    void slotOnLODGenTimer();
};

#endif // MAINWINDOW_H
