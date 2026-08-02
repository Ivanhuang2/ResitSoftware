cmake_minimum_required(VERSION 3.5)

project(VRBaseStation LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# Also link to VTK
find_package( VTK REQUIRED )

find_package(Qt6 COMPONENTS Widgets REQUIRED)

add_executable(VRBaseStation
        main.cpp
        mainwindow.cpp
        mainwindow.h
		ModelPart.cpp
		ModelPart.h
		ModelPartList.cpp
		ModelPartList.h
		optionDialog.cpp
		optionDialog.h
        VRRenderThread.cpp
        VRRenderThread.h
        mainwindow.ui
		optiondialog.ui
)

target_link_libraries(VRBaseStation PRIVATE Qt6::Widgets ${VTK_LIBRARIES})



# Copy across Open VR bindings that map controllers
# The program will expect to find these in the build dir when it runs
add_custom_target(	VRBindings )
add_custom_command(	TARGET VRBindings PRE_BUILD
		COMMAND ${CMAKE_COMMAND} -E
		copy_directory ${CMAKE_SOURCE_DIR}/vrbindings ${CMAKE_BINARY_DIR}/ )


# Specify that VRBaseStation requires VRBindings to be built
add_dependencies( VRBaseStation VRBindings )
