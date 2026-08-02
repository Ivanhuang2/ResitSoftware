#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <QMainWindow>

#include <vtkRenderer.h>
#include <vtkGenericOpenGLRenderWindow.h>

#define USE_GUI_RENDERER

#include "ModelPartList.h"
#include "VRRenderThread.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void updateRenderFromTree(const QModelIndex& index);
    void updateRender();

    void addActorsToVR();
    void addActorsToVR_recursive(const QModelIndex& index);

private:
    Ui::MainWindow *ui;
    ModelPartList* partList;
    //vtkSmartPointer<vtkRenderer> renderer;
    //vtkSmartPointer<vtkGenericOpenGLRenderWindow> renderWindow;
    //VRRenderThread* renderThread;


private slots:
    void editSelectedItem();
    void addNewItem();
};
#endif // MAINWINDOW_H
