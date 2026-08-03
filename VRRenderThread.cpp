/**		@file VRRenderThread.cpp
  *
  *		EEEE2046 - Software Engineering & VR Project
  *
  *		Template to add VR rendering to your application.
  *
  *		P Evans 2022
  */

#include "VRRenderThread.h"


/* Vtk headers */
#include <vtkActor.h>
#include <vtkOpenVRRenderWindow.h>
#include <vtkOpenVRRenderWindowInteractor.h>
#include <vtkOpenVRRenderer.h>
#include <vtkOpenVRCamera.h>

#include <vtkNew.h>
#include <vtkSmartPointer.h>
#include <vtkNamedColors.h>
#include <vtkCylinderSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkSTLReader.h>
#include <vtkDataSetMapper.h>
#include <vtkCallbackCommand.h>

/* Qt headers */
#include <QMutexLocker>


/* The class constructor is called by MainWindow and runs in the primary program thread, this thread
 * will go on to handle the GUI (mouse clicks, etc). The OpenVRRenderWindowInteractor cannot be start()ed
 * in the constructor, as it will take control of the main thread to handle the VR interaction (headset
 * rotation etc. This means that a second thread is needed to handle the VR.
 */
VRRenderThread::VRRenderThread( QObject* parent ) : QThread( parent ) {
	/* Initialise actor list.
	 * Note the use of vtkSmartPointer<>::New() rather than assigning the result
	 * of vtkActorCollection::New() - the latter leaks a reference every time,
	 * which matters because this object is created and destroyed on every
	 * VR start/stop cycle. */
	actors = vtkSmartPointer<vtkActorCollection>::New();

	/* Initialise command variables */
	rotateX = 0.;
	rotateY = 0.;
	rotateZ = 0.;

	/* Must start false - if this is left uninitialised a stray non-zero value
	 * makes run() exit immediately the first time VR is started. */
	endRender = false;
	initFailed = false;
}


/* Standard destructor - this is important here as the class will be destroyed when the user
 * stops the VR thread, and recreated when the user starts it again. If class variables are
 * not deallocated properly then there will be a memory leak, where the program's total memory
 * usage will increase for each start/stop thread cycle.
 *
 * Every VTK member is a vtkSmartPointer, so the references they hold are released
 * automatically here. The VR objects themselves are already released at the end of
 * run() - deliberately, because they must not be destroyed from the GUI thread.
 */
VRRenderThread::~VRRenderThread() {
	actors = nullptr;
}


void VRRenderThread::addActorOffline( vtkActor* actor ) {

	/* Check to see if render thread is running */
	if (!this->isRunning() && actor != nullptr) {
		double* ac = actor->GetOrigin();

		/* I have found that these initial transforms will position the FS
		 * car model in a sensible position but you can experiment
		 */
		actor->RotateX(-90);
		actor->AddPosition(-ac[0]+0, -ac[1]-100, -ac[2]-200);

		actors->AddItem(actor);
	}
}



void VRRenderThread::issueCommand( int cmd, double value ) {

	/* The GUI thread calls this while the VR thread may be reading these
	 * variables, so every access has to be under the mutex. */
	QMutexLocker locker(&mutex);

	/* Update class variables according to command */
	switch (cmd) {
		/* These are just a few basic examples */
		case END_RENDER:
			this->endRender = true;
			break;

		case ROTATE_X:
			this->rotateX = value;
			break;

		case ROTATE_Y:
			this->rotateY = value;
			break;

		case ROTATE_Z:
			this->rotateZ = value;
			break;
	}
}


bool VRRenderThread::initialisationFailed() {
	QMutexLocker locker(&mutex);
	return initFailed;
}


/* This function runs in a separate thread. This means that the program
 * can fork into two separate execution paths. This thread is triggered by
 * calling VRRenderThread::start()
 */
void VRRenderThread::run() {
	/* You might want to edit the 3D model once VR has started, however VTK is not "thread safe".
	 * This means if you try to edit the VR model from the GUI thread while the VR thread is
	 * running, the program could become corrupted and crash. The solution is to get the VR thread
	 * to edit the model. Any decision to change the VR model will come fromthe user via the GUI thread,
	 * so there needs to be a mechanism to pass data from the GUi thread to the VR thread.
	 */

	vtkNew<vtkNamedColors> colors;

	// Set the background color.
	std::array<unsigned char, 4> bkg{ {26, 51, 102, 255} };
	colors->SetColor("BkgColor", bkg.data());

	// The renderer generates the image
	// which is then displayed on the render window.
	// It can be thought of as a scene to which the actor is added
	renderer = vtkSmartPointer<vtkOpenVRRenderer>::New();

	renderer->SetBackground(colors->GetColor3d("BkgColor").GetData());

	/* Loop through list of actors provided and add to scene */
	vtkActor* a;
	actors->InitTraversal();
	while( (a = (vtkActor*)actors->GetNextActor() ) ) {
		renderer->AddActor(a);
	}

	/* The render window is the actual GUI window
	 * that appears on the computer screen
	 */
	window = vtkSmartPointer<vtkOpenVRRenderWindow>::New();

	window->Initialize();

	/* Initialize() does not throw or return a status - if SteamVR is not
	 * running, or no headset is connected, it reports the problem and simply
	 * returns without setting VRInitialized. Checking this is what stops the
	 * program hanging or crashing when the hardware is not there. */
	if (!window->GetVRInitialized()) {
		{
			QMutexLocker locker(&mutex);
			initFailed = true;
		}

		window->Finalize();

		window = nullptr;
		renderer = nullptr;
		return;
	}

	window->AddRenderer(renderer);

	/* Create Open VR Camera */
	camera = vtkSmartPointer<vtkOpenVRCamera>::New();
	renderer->SetActiveCamera(camera);

	/* The render window interactor captures mouse events
	 * and will perform appropriate camera or actor manipulation
	 * depending on the nature of the events.
	 */
	interactor = vtkSmartPointer<vtkOpenVRRenderWindowInteractor>::New();
	interactor->SetRenderWindow(window);
	interactor->Initialize();
	window->Render();


	/* Now start the VR - we will implement the command loop manually
	 * so it can be interrupted to make modifications to the actors
	 * (i.e. to implement animation)
	 *
	 * Note that endRender is NOT reset here. The constructor already set it
	 * false, and resetting it at this point would throw away a stop request
	 * that arrived while the headset was still starting up.
	 */
	t_last = std::chrono::steady_clock::now();

	for( ;; ) {
		/* Take a snapshot of everything shared with the GUI thread, so the
		 * mutex is held for as short a time as possible. */
		bool   end;
		double rx, ry, rz;
		{
			QMutexLocker locker(&mutex);
			end = endRender;
			rx  = rotateX;
			ry  = rotateY;
			rz  = rotateZ;
		}

		if (end || interactor->GetDone())
			break;

		interactor->DoOneEvent( window, renderer );

		/* Check to see if enough time has elapsed since last update
		 * This looks overcomplicated (and it is, C++ loves to make things unecessarily complicated!) but
		 * is really just checking if more than 20ms have elaspsed since the last animation step. The
		 * complications comes from the fact that numbers representing time on computers don't usually have
		 * standard second/millisecond units. Because everything is a class in C++, the converion from
		 * computer units to seconds/milliseconds ends up looking like what you see below.
		 *
		 * My choice of 20ms is arbitrary, if this value is too small the animation calculations could begin to
		 * interfere with the interator processes and make the simulation unresponsive. If it is too large
		 * the animations will be jerky. Play with the value to see what works best.
		 */
		if (std::chrono::duration_cast <std::chrono::milliseconds> (std::chrono::steady_clock::now() - t_last).count() > 20) {

			/* Do things that might need doing ... */
			vtkActorCollection* actorList = renderer->GetActors();
			vtkActor* actorToRotate;

			/* X Rotation */
			actorList->InitTraversal();
			while ((actorToRotate = (vtkActor*)actorList->GetNextActor())) {
				actorToRotate->RotateX(rx);
			}

			/* Y Rotation */
			actorList->InitTraversal();
			while ((actorToRotate = (vtkActor*)actorList->GetNextActor())) {
				actorToRotate->RotateY(ry);
			}

			/* Z Rotation */
			actorList->InitTraversal();
			while ((actorToRotate = (vtkActor*)actorList->GetNextActor())) {
				actorToRotate->RotateZ(rz);
			}

			/* Remember time now */
			t_last = std::chrono::steady_clock::now();
		}
	}

	/* Shut the VR system down properly, and do it HERE - in the thread that
	 * created these objects. Finalize() releases the OpenVR runtime and the
	 * OpenGL resources; releasing the smart pointers afterwards destroys the
	 * objects while we are still on the correct thread.
	 *
	 * This is what allows VR to be stopped without crashing the program. If
	 * these objects were left for the destructor to clean up they would be
	 * destroyed from the GUI thread, which is exactly what VTK forbids. */
	window->Finalize();

	interactor = nullptr;
	camera     = nullptr;
	renderer   = nullptr;
	window     = nullptr;
}
