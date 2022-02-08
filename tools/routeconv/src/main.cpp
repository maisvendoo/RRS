#include    "mainwindow.h"
#include    <QApplication>
#include    <QTranslator>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator translator;

    if (translator.load(QLocale(),
                        QLatin1String("routeconv"),
                        QLatin1String("."),
                        QLatin1String(":/translations/translations"),
                        QLatin1String(".qm")))

        a.installTranslator(&translator);

    MainWindow w;
    w.show();

    return a.exec();
}
