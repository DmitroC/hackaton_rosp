#include "widget.h"

#include <QApplication>
#include <QPushButton>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    theScreen ts;
    ts.resize(500, 500);
    ts.show();
    return a.exec();
}
