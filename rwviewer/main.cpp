#include <QApplication>
#include "ViewerWindow.hpp"

int main(int argc, char* argv[]) {
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    QApplication app(argc, argv);

    ViewerWindow viewer;

    return app.exec();
}
