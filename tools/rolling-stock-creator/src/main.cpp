#include "mainwindow.h"

#include <QApplication>

//------------------------------------------------------------------------------
//
//  Редактор характеристик подвижного состава
//  (ТЗ «ПРОГА-ЭКСПОРТЕР и редактор характеристик ПС», вкладка характеристик)
//
//------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    QApplication::setOrganizationName("RRS");
    QApplication::setApplicationName("rolling-stock-creator");

    MainWindow window;
    window.show();

    return app.exec();
}
