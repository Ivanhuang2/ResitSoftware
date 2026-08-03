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


#-----------------------------------------------------------------------------
# Installation
#-----------------------------------------------------------------------------

# Work out where the runtime pieces live, so the installer can gather them up.

# VTK's DLLs sit in <prefix>/bin, and VTK_DIR points at <prefix>/lib/cmake/vtk-X.Y
get_filename_component( VTK_RUNTIME_DIR "${VTK_DIR}/../../../bin" ABSOLUTE )

# Qt's DLLs and windeployqt live alongside qmake
get_target_property( QMAKE_EXECUTABLE Qt6::qmake IMPORTED_LOCATION )
get_filename_component( QT_BIN_DIR "${QMAKE_EXECUTABLE}" DIRECTORY )
find_program( WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${QT_BIN_DIR}" )

# The OpenVR SDK ships openvr_api.dll separately from the .lib we link against.
# The application cannot start without it, so it has to be installed even though
# the brief does not require bundling the other libraries.
get_filename_component( OPENVR_LIB_DIR "${OpenVR_LIBRARY}" DIRECTORY )
get_filename_component( OPENVR_ROOT_DIR "${OPENVR_LIB_DIR}/../.." ABSOLUTE )
set( OpenVR_RUNTIME_DLL "${OPENVR_ROOT_DIR}/bin/win64/openvr_api.dll"
	CACHE FILEPATH "Full path to openvr_api.dll, installed next to the executable" )
get_filename_component( OPENVR_BIN_DIR "${OpenVR_RUNTIME_DLL}" DIRECTORY )


# The program itself. RUNTIME_DEPENDENCY_SET makes CMake walk the executable's
# DLL imports so the VTK and Qt libraries it actually uses get pulled in too.
install( TARGETS VRBaseStation
	RUNTIME_DEPENDENCY_SET VRBaseStationDeps
	RUNTIME DESTINATION bin )

install( RUNTIME_DEPENDENCY_SET VRBaseStationDeps
	PRE_EXCLUDE_REGEXES "api-ms-win-.*" "ext-ms-.*"
	POST_EXCLUDE_REGEXES ".*[Ss]ystem32.*"
	DIRECTORIES "${VTK_RUNTIME_DIR}" "${QT_BIN_DIR}" "${OPENVR_BIN_DIR}"
	DESTINATION bin )

# OpenVR controller bindings, read by the program at runtime
install( DIRECTORY "${CMAKE_SOURCE_DIR}/vrbindings/" DESTINATION bin )

# openvr_api.dll, installed explicitly rather than relying on the dependency
# scan, because without it the program will not even start
if( EXISTS "${OpenVR_RUNTIME_DLL}" )
	install( FILES "${OpenVR_RUNTIME_DLL}" DESTINATION bin )
else()
	message( WARNING "openvr_api.dll not found at ${OpenVR_RUNTIME_DLL} - "
		"the installed program will not start. Set OpenVR_RUNTIME_DLL." )
endif()

# The Visual C++ runtime, so the program runs on a machine without Visual Studio
set( CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION bin )
include( InstallRequiredSystemLibraries )

# Qt needs more than its DLLs - the platform plugin in particular - and only
# windeployqt knows the full list. Run it over the staged executable.
if( WINDEPLOYQT_EXECUTABLE )
	install( CODE "
		message( STATUS \"Running windeployqt...\" )
		execute_process( COMMAND \"${WINDEPLOYQT_EXECUTABLE}\"
			--release --no-translations --no-compiler-runtime
			\"\${CMAKE_INSTALL_PREFIX}/bin/VRBaseStation.exe\" )
	" )
else()
	message( WARNING "windeployqt not found - the installed program will be missing Qt plugins." )
endif()


#-----------------------------------------------------------------------------
# Packaging (CPack / NSIS)
#-----------------------------------------------------------------------------

set( CPACK_PACKAGE_NAME               "VRBaseStation" )
set( CPACK_PACKAGE_VENDOR             "University of Nottingham - EEEE2076" )
set( CPACK_PACKAGE_DESCRIPTION_SUMMARY "VR Base Station - STL viewer with VR support" )
set( CPACK_PACKAGE_VERSION_MAJOR      1 )
set( CPACK_PACKAGE_VERSION_MINOR      0 )
set( CPACK_PACKAGE_VERSION_PATCH      0 )
set( CPACK_PACKAGE_INSTALL_DIRECTORY  "VRBaseStation" )

set( CPACK_GENERATOR "NSIS" )

set( CPACK_NSIS_DISPLAY_NAME "VR Base Station" )
set( CPACK_NSIS_PACKAGE_NAME "VR Base Station" )
set( CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON )
set( CPACK_NSIS_MODIFY_PATH OFF )

# Start menu entry, and the matching desktop shortcut
set( CPACK_PACKAGE_EXECUTABLES "VRBaseStation" "VR Base Station" )
set( CPACK_CREATE_DESKTOP_LINKS "VRBaseStation" )

include( CPack )
