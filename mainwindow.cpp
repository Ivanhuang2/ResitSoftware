/**     @file mainwindow.cpp
  *
  *     EEEE2076 - Software Engineering & VR Project
  *
  *     Main application window: menubar, toolbar, model treeview, VTK render
  *     widget and status bar.
  */

/* Project headers */
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "optiondialog.h"

/* Qt headers */
#include <QMessageBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QFileInfoList>
#include <QPushButton>
#include <QStatusBar>
#include <QSplitter>
#include <QTreeView>
#include <QDir>
#include <QStringList>

/* Vtk headers */
#include <vtkSmartPointer.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkCamera.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkSTLReader.h>


/** Name filters used when scanning for STL files. Both cases are listed so
  * the behaviour is the same on case-sensitive file systems. */
static const QStringList kStlFilters = { QStringLiteral("*.stl"), QStringLiteral("*.STL") };


/** Test whether a directory, or any directory below it, contains an STL file.
  * Used so that Open Directory only creates branches for sub-directories that
  * actually have something in them.
  * @param dir is the directory to test
  * @return true if at least one STL file was found
  */
static bool directoryContainsSTL(const QDir& dir) {
    if (!dir.entryList(kStlFilters, QDir::Files).isEmpty())
        return true;

    const QFileInfoList subDirs = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QFileInfo& sub : subDirs) {
        if (directoryContainsSTL(QDir(sub.absoluteFilePath())))
            return true;
    }

    return false;
}


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , partList(nullptr)
    , renderThread(nullptr)
{
	ui->setupUi(this);


    /* Link TreeView to Model */
    this->partList = new ModelPartList("PartsList");
    ui->treeView->setModel(this->partList);

    /* Create a root item in the tree. Everything the user loads is added
     * below this item. */
    QModelIndex root;
    partList->appendChild(root, { QVariant("Model"), QVariant("true")});
    ui->treeView->expandAll();

    /* The treeview's context menu policy is set to ActionsContextMenu in the
     * .ui file, which means any QAction added to the widget appears when the
     * user right-clicks it. */
    ui->treeView->addAction(ui->actionItemOptions);

    /* Give the 3D view the majority of the width, but let the user drag the
     * divider to whatever suits them. */
    ui->splitter->setStretchFactor(0, 0);
    ui->splitter->setStretchFactor(1, 1);
    ui->splitter->setSizes({ 260, 740 });


    /* Setup Renderer */
    renderer = vtkSmartPointer<vtkRenderer>::New();
    renderWindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    renderWindow->AddRenderer(renderer);
    ui->vtkWidget->setRenderWindow(renderWindow);

    renderer->SetBackground(0.10, 0.20, 0.40);
    renderer->GetActiveCamera()->Azimuth(30);
    renderer->GetActiveCamera()->Elevation(30);
    renderer->ResetCamera();


    /* Connect up the menu / toolbar actions */
    connect(ui->actionOpenFile,      &QAction::triggered, this, &MainWindow::handleOpenFile);
    connect(ui->actionOpenDirectory, &QAction::triggered, this, &MainWindow::handleOpenDirectory);
    connect(ui->actionItemOptions,   &QAction::triggered, this, &MainWindow::editSelectedItem);
    connect(ui->actionStartVR,       &QAction::triggered, this, &MainWindow::handleStartVR);
    connect(ui->actionStopVR,        &QAction::triggered, this, &MainWindow::handleStopVR);
    connect(ui->actionAbout,         &QAction::triggered, this, &MainWindow::handleAbout);
    connect(ui->actionQuit,          &QAction::triggered, this, &MainWindow::close);

    connect(ui->treeView, &QTreeView::clicked, this, &MainWindow::handleTreeClicked);

    /* VR cannot be stopped until it has been started */
    ui->actionStopVR->setEnabled(false);

    ui->statusbar->showMessage(tr("Ready. Use File > Open File or File > Open Directory to load STL models."));
}

MainWindow::~MainWindow()
{
    /* If the user closes the window while VR is still running, ask the thread
     * to finish and wait for it before tearing anything down. */
    if (renderThread) {
        renderThread->issueCommand(VRRenderThread::END_RENDER, 0.0);
        renderThread->wait();
        delete renderThread;
        renderThread = nullptr;
    }

    delete ui;
}


QModelIndex MainWindow::modelRoot() {
    return partList->index(0, 0, QModelIndex());
}


void MainWindow::addNewItem() {
    // Get index of currently selected item
    QModelIndex index = ui->treeView->currentIndex();

    // Add a child
    QModelIndex childIndex = partList->appendChild(index, { QVariant("New"), QVariant("true") });

    ui->treeView->expand(index);
    ui->treeView->setCurrentIndex(childIndex);
}


void MainWindow::handleOpenFile() {
    const QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("Open STL File"),
        QString(),
        tr("STL Files (*.stl);;All Files (*)"));

    if (fileName.isEmpty()) {
        ui->statusbar->showMessage(tr("Open File cancelled."), 3000);
        return;
    }

    const QFileInfo info(fileName);

    QModelIndex parentIndex = modelRoot();
    QModelIndex newIndex = partList->appendChild(parentIndex,
                                                 { QVariant(info.fileName()), QVariant("true") });

    ModelPart* part = static_cast<ModelPart*>(newIndex.internalPointer());

    if (!part || !part->loadSTL(fileName)) {
        QMessageBox::warning(this,
                             tr("Could not load file"),
                             tr("'%1' could not be read as an STL file.").arg(info.fileName()));
        ui->statusbar->showMessage(tr("Failed to load %1").arg(info.fileName()), 5000);
        return;
    }

    ui->treeView->expandAll();
    ui->treeView->setCurrentIndex(newIndex);

    updateRender(true);

    ui->statusbar->showMessage(tr("Loaded %1").arg(info.fileName()));
}


void MainWindow::handleOpenDirectory() {
    const QString dirName = QFileDialog::getExistingDirectory(
        this,
        tr("Open Directory"),
        QString(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (dirName.isEmpty()) {
        ui->statusbar->showMessage(tr("Open Directory cancelled."), 3000);
        return;
    }

    const QDir dir(dirName);

    if (!directoryContainsSTL(dir)) {
        QMessageBox::information(this,
                                 tr("No STL files"),
                                 tr("'%1' does not contain any STL files.").arg(dir.dirName()));
        ui->statusbar->showMessage(tr("No STL files found in %1").arg(dir.dirName()), 5000);
        return;
    }

    /* The chosen directory itself becomes a branch, so several directories can
     * be opened one after another without their contents getting mixed up. */
    QModelIndex parentIndex = modelRoot();
    QModelIndex dirIndex = partList->appendChild(parentIndex,
                                                 { QVariant(dir.dirName()), QVariant("true") });

    const int loaded = loadDirectory(dir, dirIndex);

    ui->treeView->expandAll();
    ui->treeView->setCurrentIndex(dirIndex);

    updateRender(true);

    ui->statusbar->showMessage(tr("Loaded %n STL file(s) from %1", "", loaded).arg(dir.dirName()));
}


int MainWindow::loadDirectory(const QDir& dir, QModelIndex& parentIndex) {
    int loaded = 0;

    /* 1. STL files sitting directly in this directory become leaf items */
    const QFileInfoList files = dir.entryInfoList(kStlFilters, QDir::Files, QDir::Name);
    for (const QFileInfo& fileInfo : files) {
        QModelIndex childIndex = partList->appendChild(parentIndex,
                                                       { QVariant(fileInfo.fileName()), QVariant("true") });

        ModelPart* part = static_cast<ModelPart*>(childIndex.internalPointer());
        if (part && part->loadSTL(fileInfo.absoluteFilePath()))
            loaded++;
    }

    /* 2. Sub-directories containing STL files appear as a new child branch.
     *    Sub-directories with nothing in them are skipped so the tree does not
     *    fill up with empty folders. */
    const QFileInfoList subDirs = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    for (const QFileInfo& subInfo : subDirs) {
        const QDir subDir(subInfo.absoluteFilePath());

        if (!directoryContainsSTL(subDir))
            continue;

        QModelIndex branchIndex = partList->appendChild(parentIndex,
                                                        { QVariant(subInfo.fileName()), QVariant("true") });

        loaded += loadDirectory(subDir, branchIndex);
    }

    return loaded;
}


void MainWindow::editSelectedItem() {

    /* Get selected item */
    QModelIndex index = ui->treeView->currentIndex();

    /* Check that something was actually selected before right click ... */
    if (!index.isValid()) {
        ui->statusbar->showMessage(tr("Select an item in the tree first."), 3000);
        return;
    }

    /* Get pointer to selected part */
    ModelPart* part = static_cast<ModelPart*>(index.internalPointer());
    if (!part)
        return;

    /* Run a dialog to change colour ... */
    OptionDialog dialog(this, part);

    if (dialog.exec() != QDialog::Accepted) {
        ui->statusbar->showMessage(tr("Item properties unchanged."), 3000);
        return;
    }

    dialog.updatePartFromDialog(part);

    /* Redraw the tree row (name / visibility may have changed) and the 3D view
     * (colour / visibility may have changed). The camera is deliberately left
     * alone so the user does not lose their viewpoint. */
    partList->refreshItem(index);
    updateRender(false);

    ui->statusbar->showMessage(tr("Updated '%1' - colour (%2, %3, %4), %5")
                                   .arg(part->data(0).toString())
                                   .arg(static_cast<int>(part->getColourR()))
                                   .arg(static_cast<int>(part->getColourG()))
                                   .arg(static_cast<int>(part->getColourB()))
                                   .arg(part->visible() ? tr("visible") : tr("hidden")));
}


void MainWindow::handleTreeClicked(const QModelIndex& index) {
    if (!index.isValid())
        return;

    ModelPart* part = static_cast<ModelPart*>(index.internalPointer());
    if (!part)
        return;

    ui->statusbar->showMessage(tr("Selected '%1' - colour (%2, %3, %4), %5")
                                   .arg(part->data(0).toString())
                                   .arg(static_cast<int>(part->getColourR()))
                                   .arg(static_cast<int>(part->getColourG()))
                                   .arg(static_cast<int>(part->getColourB()))
                                   .arg(part->visible() ? tr("visible") : tr("hidden")));
}


void MainWindow::handleStartVR() {
    if (renderThread && renderThread->isRunning()) {
        ui->statusbar->showMessage(tr("VR is already running."), 3000);
        return;
    }

    /* Dispose of the thread object left over from a previous session. It has
     * already finished, so this does not block. */
    if (renderThread) {
        renderThread->wait();
        delete renderThread;
        renderThread = nullptr;
    }

    renderThread = new VRRenderThread(this);
    connect(renderThread, &QThread::finished, this, &MainWindow::handleVRFinished);

    /* Actors can only be handed over before the thread starts, so this has to
     * happen here rather than after start(). */
    const int actorCount = addActorsToVR();

    if (actorCount == 0) {
        QMessageBox::information(this,
                                 tr("Nothing to show"),
                                 tr("Load at least one STL model before starting VR."));
        ui->statusbar->showMessage(tr("VR not started - no models loaded."), 5000);

        delete renderThread;
        renderThread = nullptr;
        return;
    }

    ui->actionStartVR->setEnabled(false);
    ui->actionStopVR->setEnabled(true);
    ui->statusbar->showMessage(tr("Starting VR with %n part(s) - make sure SteamVR is running...", "", actorCount));

    renderThread->start();
}


void MainWindow::handleStopVR() {
    if (!renderThread || !renderThread->isRunning()) {
        ui->statusbar->showMessage(tr("VR is not running."), 3000);
        return;
    }

    /* Ask the VR thread to leave its render loop. It shuts the headset down and
     * exits on its own, and handleVRFinished() tidies up the GUI afterwards -
     * so the GUI is never blocked waiting here. */
    ui->actionStopVR->setEnabled(false);
    ui->statusbar->showMessage(tr("Stopping VR..."));

    renderThread->issueCommand(VRRenderThread::END_RENDER, 0.0);
}


void MainWindow::handleVRFinished() {
    ui->actionStartVR->setEnabled(true);
    ui->actionStopVR->setEnabled(false);

    if (renderThread && renderThread->initialisationFailed()) {
        ui->statusbar->showMessage(tr("VR could not start - check SteamVR and the headset."), 8000);
        QMessageBox::warning(this,
                             tr("VR unavailable"),
                             tr("The VR system could not be initialised.\n\n"
                                "Check that SteamVR is running and that the headset "
                                "is connected and tracking, then try again."));
        return;
    }

    ui->statusbar->showMessage(tr("VR stopped."), 5000);
}


void MainWindow::handleAbout() {
    QMessageBox::about(this,
                       tr("About VR Base Station"),
                       tr("<b>VR Base Station - STL Viewer</b><br><br>"
                          "EEEE2076 Software Engineering &amp; VR Project.<br>"
                          "Loads STL models into a tree, renders them with VTK "
                          "and displays them on an HTC Vive headset."));
}


/* These two functions can be used to add all items in the tree view to the VTK view */
void MainWindow::updateRender(bool resetCamera) {

    // Remove all items from VTK Renderer
    renderer->RemoveAllViewProps();

    /* Walk every top level branch of the tree */
    const int rows = partList->rowCount(QModelIndex());
    for (int i = 0; i < rows; i++) {
        updateRenderFromTree(partList->index(i, 0, QModelIndex()));
    }

    if (resetCamera)
        renderer->ResetCamera();

    renderWindow->Render();
}


void MainWindow::updateRenderFromTree( const QModelIndex& index ){

    if( index.isValid() ) {
        /* Get item at this stage of the tree */
        ModelPart* selectedPart = static_cast<ModelPart*>(index.internalPointer());

        /* Add it to the VTK renderer.
         * Branch items (folders) have no actor, so the null check matters. */
        if (selectedPart) {
            vtkSmartPointer<vtkActor> actor = selectedPart->getActor();
            if (actor)
                renderer->AddActor(actor);
        }
    }


    if( !partList->hasChildren(index) || (index.flags() & Qt::ItemNeverHasChildren) )
    {
        return;
    }

    int rows = partList->rowCount( index );
    for (int i = 0; i < rows; i++) {
        updateRenderFromTree(partList->index(i, 0, index));
    }

}


/* These two functions can be used to add all items in the tree view to the VR view */
int MainWindow::addActorsToVR() {
    int added = 0;

    const int rows = partList->rowCount(QModelIndex());
    for (int i = 0; i < rows; i++) {
        added += addActorsToVR_recursive(partList->index(i, 0, QModelIndex()));
    }

    return added;
}


int MainWindow::addActorsToVR_recursive(const QModelIndex& index)
{
    int added = 0;

    if (index.isValid()) {
        /* Get item at this stage of the tree */
        ModelPart* selectedPart = static_cast<ModelPart*>(index.internalPointer());

        /* Add it to the VR renderer. getNewActor() builds a second actor over
         * the same geometry, because an actor cannot belong to two renderers.
         * Branch items (folders) have no geometry and return null. */
        if (selectedPart && renderThread) {
            vtkSmartPointer<vtkActor> vrActor = selectedPart->getNewActor();
            if (vrActor) {
                /* The collection takes its own reference, so it is safe to let
                 * this smart pointer go out of scope afterwards. */
                renderThread->addActorOffline(vrActor.Get());
                added++;
            }
        }
    }

    if (!partList->hasChildren(index) || (index.flags() & Qt::ItemNeverHasChildren)) {
        return added;
    }


    int rows = partList->rowCount(index);
    for (int i = 0; i < rows; i++) {
        added += addActorsToVR_recursive(partList->index(i, 0, index));
    }

    return added;
}
