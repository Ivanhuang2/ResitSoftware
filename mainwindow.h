/**     @file mainwindow.h
  *
  *     EEEE2076 - Software Engineering & VR Project
  *
  *     Main application window: menubar, toolbar, model treeview, VTK render
  *     widget and status bar.
  */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <QMainWindow>
#include <QModelIndex>
#include <QString>
#include <QDir>

#include <vtkSmartPointer.h>
#include <vtkRenderer.h>
#include <vtkGenericOpenGLRenderWindow.h>

#include "ModelPartList.h"
#include "VRRenderThread.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

/** @class MainWindow
  * @brief The application's main window.
  *
  * Owns the model tree (ModelPartList), the VTK renderer that draws it in the
  * GUI, and the VR render thread. Handles the Open File / Open Directory
  * actions, the right-click colour dialog, and keeps the status bar updated.
  */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /** Constructor - builds the UI, creates the model and the VTK pipeline
      * and wires up all the actions.
      * @param parent is the parent widget (normally nullptr for a main window)
      */
    MainWindow(QWidget *parent = nullptr);

    /** Destructor - stops the VR thread if it is running and frees the UI */
    ~MainWindow();

    /** Walk one branch of the tree and add every part's actor to the GUI renderer.
      * Recurses into child items.
      * @param index is the tree item to start from
      */
    void updateRenderFromTree(const QModelIndex& index);

    /** Rebuild the whole VTK scene from the current contents of the tree and
      * redraw the render window.
      * @param resetCamera if true the camera is re-framed around the model;
      *        pass false when only appearance changed so the user's viewpoint is kept
      */
    void updateRender(bool resetCamera = false);

    /** Add every part in the tree to the VR renderer.
      * @return the number of actors handed to the VR thread
      */
    int addActorsToVR();

    /** Walk one branch of the tree and add every part's VR actor to the VR thread.
      * Recurses into child items.
      * @param index is the tree item to start from
      * @return the number of actors added from this branch
      */
    int addActorsToVR_recursive(const QModelIndex& index);

private:
    /** Get the index of the top level "Model" item that everything hangs from.
      * @return index of the first top level row
      */
    QModelIndex modelRoot();

    /** Recursively add the contents of a directory to the tree.
      * STL files in the directory become leaf items; sub-directories that
      * contain STL files (at any depth) become child branches.
      * @param dir is the directory to scan
      * @param parentIndex is the tree item the contents should be added below
      * @return the number of STL files successfully loaded
      */
    int loadDirectory(const QDir& dir, QModelIndex& parentIndex);

    Ui::MainWindow *ui;                                         /**< Pointer to the widgets generated from mainwindow.ui */
    ModelPartList* partList;                                    /**< Item model that backs the treeview */
    vtkSmartPointer<vtkRenderer> renderer;                      /**< Renderer for the GUI 3D view */
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> renderWindow; /**< Render window hosted by the Qt VTK widget */
    VRRenderThread* renderThread;                               /**< Background thread that drives the VR headset, nullptr when VR is not running */


private slots:
    /** Open the property dialog for the currently selected tree item and apply
      * any changes the user makes. Triggered by right-clicking the treeview. */
    void editSelectedItem();

    /** Add a placeholder child item below the current selection. */
    void addNewItem();

    /** Prompt for a single STL file, add it to the tree and render it. */
    void handleOpenFile();

    /** Prompt for a directory and add all the STL files it contains to the tree. */
    void handleOpenDirectory();

    /** Start the VR render thread. */
    void handleStartVR();

    /** Stop the VR render thread. */
    void handleStopVR();

    /** Restore the VR menu/toolbar state once the VR thread has exited, and
      * report whether it stopped normally or never managed to start. */
    void handleVRFinished();

    /** Show the about box. */
    void handleAbout();

    /** Report the properties of a clicked tree item on the status bar.
      * @param index is the item that was clicked
      */
    void handleTreeClicked(const QModelIndex& index);
};
#endif // MAINWINDOW_H
