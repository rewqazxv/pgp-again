#include "ui/main-window.h"


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator translations(&a);
    if (translations.load(QLocale::system(), "", "", ":/translations"))
        a.installTranslator(&translations);

    MainWindow w;
    w.show();

    return a.exec();
}
