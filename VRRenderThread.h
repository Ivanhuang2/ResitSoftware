/**		@file VRRenderThread.h
  *
  *		EEEE2046 - Software Engineering & VR Project
  *
  *		Template to add VR rendering to your application.
  *
  *		P Evans 2022
  */
#ifndef VR_RENDER_THREAD_H
#define VR_RENDER_THREAD_H

/* Project headers */

/* Standard headers */
#include <chrono>

/* Qt headers */
#include <QThread>
#include <QMutex>
#include <QWaitCondition>

/* Vtk headers */
#include <vtkActor.h>
#include <vtkOpenVRRenderWindow.h>
#include <vtkOpenVRRenderWindowInteractor.h>
#include <vtkOpenVRRenderer.h>
#include <vtkOpenVRCamera.h>
#include <vtkActorCollection.h>
#include <vtkCommand.h>



/** @class VRRenderThread
  * @brief Background thread that drives the OpenVR render loop.
  *
  * Note that this class inherits from the Qt class QThread which allows it to be a parallel thread
  * to the main() thread, and also from vtkCommand which allows it to act as a "callback" for the
  * vtkRenderWindowInteractor. This callback functionallity means that once the renderWindowInteractor
  * takes control of this thread to enable VR, it can callback to a function in the class to check to see
  * if the user has requested any changes
  *
  * All of the VTK VR objects are created, used and destroyed inside run(), i.e. inside
  * this thread. That matters: VTK is not thread safe, and destroying the VR render
  * window from the GUI thread is what makes stopping VR crash.
  */
class VRRenderThread : public QThread {
    Q_OBJECT

public:
    /** List of command names that can be passed to issueCommand() */
    enum Command {
        END_RENDER,     /**< Ask the render loop to finish and shut VR down cleanly */
        ROTATE_X,       /**< Set the per-timestep rotation about the X axis */
        ROTATE_Y,       /**< Set the per-timestep rotation about the Y axis */
        ROTATE_Z        /**< Set the per-timestep rotation about the Z axis */
    };


    /**  Constructor
      * @param parent is the Qt object that will own this thread
      */
    VRRenderThread(QObject* parent = nullptr);

    /**  Denstructor
      */
    ~VRRenderThread();

    /** This allows actors to be added to the VR renderer BEFORE the VR
      * interactor has been started
      * @param actor is the actor to add; the collection takes its own
      *        reference, so the caller may release theirs afterwards
     */
    void addActorOffline(vtkActor* actor);


    /** This allows commands to be issued to the VR thread in a thread safe way.
      * Function will set variables within the class to indicate the type of
      * action / animation / etc to perform. The rendering thread will then impelement this.
      * @param cmd is one of the Command enumerators
      * @param value is the value that goes with the command (e.g. degrees of rotation)
      */
    void issueCommand( int cmd, double value );


    /** Report whether the VR system failed to start up.
      * Only meaningful once the thread has finished. A true result normally
      * means SteamVR was not running or no headset was connected.
      * @return true if OpenVR could not be initialised
      */
    bool initialisationFailed();


protected:
    /** This is a re-implementation of a QThread function
      */
    void run() override;

private:
    /* Standard VTK VR Classes */
    vtkSmartPointer<vtkOpenVRRenderWindow>              window;     /**< VR render window, only valid while the thread runs */
    vtkSmartPointer<vtkOpenVRRenderWindowInteractor>    interactor; /**< Handles headset and controller input */
    vtkSmartPointer<vtkOpenVRRenderer>                  renderer;   /**< Scene that the VR actors are added to */
    vtkSmartPointer<vtkOpenVRCamera>                    camera;     /**< VR camera driven by the headset pose */

    /* Use to synchronise passing of data to VR thread */
    QMutex                                              mutex;      /**< Guards every variable shared with the GUI thread */
    QWaitCondition                                      condition;  /**< Available for blocking hand-off between threads */

    /** List of actors that will need to be added to the VR scene */
    vtkSmartPointer<vtkActorCollection>                 actors;

    /** A timer to help implement animations and visual effects */
    std::chrono::time_point<std::chrono::steady_clock>  t_last;

    /** This will be set to false by the constructor, if it is set to true
      * by the GUI then the rendering will end
      */
    bool                                                endRender;

    /** Set by run() if OpenVR could not be initialised. Guarded by mutex. */
    bool                                                initFailed;

    /* Some variables to indicate animation actions to apply.
     *
     */
    double rotateX;         /*< Degrees to rotate around X axis (per time-step) */
    double rotateY;         /*< Degrees to rotate around Y axis (per time-step) */
    double rotateZ;         /*< Degrees to rotate around Z axis (per time-step) */
};



#endif
