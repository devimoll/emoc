#include "emoc.hpp"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setApplicationName("emoc");
    QApplication::setApplicationVersion("0.9");
    emoc w;
    w.show();
    return a.exec();
}
