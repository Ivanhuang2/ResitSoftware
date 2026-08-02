/**     @file main.cpp
  *
  *     EEEE2076 - Software Engineering & VR Project
  *
  *     Program entry point for the VR Base Station STL viewer.
  *
  *     Ivan Huang
  */

#include "mainwindow.h"

#include <QApplication>
#include <QSurfaceFormat>

#include <QVTKOpenGLNativeWidget.h>

/** Program entry point.
  *
  * Sets up the default OpenGL surface format required by VTK's Qt widget,
  * creates the application object and the main window, then hands control
  * over to the Qt event loop.
  *
  * @param argc is the number of command line arguments
  * @param argv is the array of command line argument strings
  * @return the exit code returned by the Qt event loop
  */
int main(int argc, char *argv[])
{
    /* This must happen BEFORE the QApplication is constructed. VTK needs a
     * specific OpenGL surface configuration (depth buffer, stencil buffer,
     * core profile); if the default format is not applied first, the
     * QVTKOpenGLNativeWidget ends up with an incompatible context and the
     * 3D view renders black. */
    QSurfaceFormat::setDefaultFormat(QVTKOpenGLNativeWidget::defaultFormat());

    QApplication a(argc, argv);

    MainWindow w;
    w.show();
    return a.exec();
}
