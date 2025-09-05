#include "mainwindow.h"
#include <QApplication>
#include <QTranslator>
#include <QDebug>

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    QLocale::setDefault(QLocale(QLocale::Russian, QLocale::Russia));

    QTranslator translator;
    if (!translator.load("qtbase_ru", "translations")) {
        qWarning() << "Cannot load translation qtbase_ru.qm !";
    }
    if (translator.load("dbschool1_ru", "translations"))
    {
        a.installTranslator(&translator);
    } else {
        qWarning() << "Cannot load Russian application tranlation";
    }

    MainWindow main;
    main.resize(1024, 768);
    main.show();
    return QApplication::exec();
}