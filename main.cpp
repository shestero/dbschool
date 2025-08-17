#include "mainwindow.h"
#include <QApplication>
#include <QTranslator>
#include <QDebug>

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    QTranslator translator;
    // Load the French translation file (myapp_fr.qm)
    if (translator.load("../translations/dbschool1_ru.qm"))
    {
        a.installTranslator(&translator);
    } else {
        qDebug() << "Cannot load russian tranlation";
    }
    MainWindow main;
    main.resize(1024, 768);
    main.show();
    return QApplication::exec();
}