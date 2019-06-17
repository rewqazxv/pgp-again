#include "pch.hpp"
#include "mainwindow.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    QTranslator translations(&a);
    translations.load(QLocale::system(), "", "", ":/translations");
    a.installTranslator(&translations);

    MainWindow w;
    w.show();

    return a.exec();
}
