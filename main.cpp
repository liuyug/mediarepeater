#include <QtGui>

#include "mainwindow.h"

int main(int argv, char **args)
{
    QApplication app(argv, args);
    app.setApplicationName("Media Repeater");
    app.setQuitOnLastWindowClosed(true);

    MainWindow window;
    window.show();
    return app.exec();
}
