cmake_minimum_required(VERSION 3.16)

project(VRBaseStation LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# Link to VTK.
# The components are listed explicitly so that CMake fails early with a clear
# message if the local VTK build is missing one of them (rather than failing
# at link time). RenderingOpenVR is what provides the vtkOpenVR* classes used
# by VRRenderThread, and GUISupportQt provides QVTKOpenGLNativeWidget.
find_package( VTK REQUIRED
	COMPONENTS
		CommonColor
		CommonCore
		CommonDataModel
		FiltersSources
		GUISupportQt
		InteractionStyle
		IOGeometry
		RenderingCore
		RenderingFreeType
		RenderingOpenGL2
		RenderingOpenVR
)

# Qt6::OpenGLWidgets is required because QVTKOpenGLNativeWidget derives from
# QOpenGLWidget, which lives in that module in Qt6.
find_package(Qt6 COMPONENTS Widgets OpenGLWidgets REQUIRED)

add_executable(VRBaseStation
        main.cpp
        mainwindow.cpp
        mainwindow.h
		ModelPart.cpp
		ModelPart.h
		ModelPartList.cpp
		ModelPartList.h
		optiondialog.cpp
		optiondialog.h
        VRRenderThread.cpp
        VRRenderThread.h
        mainwindow.ui
		optiondialog.ui
		icons.qrc
)

target_link_libraries(VRBaseStation PRIVATE
	Qt6::Widgets
	Qt6::OpenGLWidgets
	${VTK_LIBRARIES}
)


# VTK 9 uses "factory" registration for its rendering back-ends. Without this
# call the object factories are never initialised and the render window comes
# up empty (or the program aborts on the first Render()). It MUST come after
# target_link_libraries.
vtk_module_autoinit(
	TARGETS VRBaseStation
	MODULES ${VTK_LIBRARIES}
)


# Copy across Open VR bindings that map controllers.
# The program expects to find these next to the executable when it runs. Note
# that multi-configuration generators (Visual Studio) put the executable in a
# per-configuration sub-directory, so the generator expression is used to get
# the real output directory rather than assuming CMAKE_BINARY_DIR.
add_custom_command( TARGET VRBaseStation POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory
		${CMAKE_SOURCE_DIR}/vrbindings
		$<TARGET_FILE_DIR:VRBaseStation>
		COMMENT "Copying OpenVR controller bindings next to the executable" )
