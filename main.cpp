#include <QApplication>
#include "include/App.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    App myApp;
    myApp.show();

    return QApplication::exec();
}