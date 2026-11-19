#include    <mainwindow.h>

#include    <QApplication>
#include    <QLocale>
#include    <QTranslator>
#include    <crash-handler.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    setup_crash_handler("../logs/dispatcher-console-crash.log");

    QApplication a(argc, argv);

    a.setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::Round);

    QTranslator translator;

    if (translator.load("dispatcher-console.ru_RU.qm", ":/translations/translations"))
    {
        a.installTranslator(&translator);
    }

    MainWindow w;
    w.show();
    return a.exec();
}