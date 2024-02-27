CMAKE_MINIMUM_REQUIRED(VERSION 3.0)
project(rpg)

# Include the engine CMakeLists for Options
include(../../engine/src/CMakeLists.txt)

# set(source_files Window.xib)
set(XIB_FILE Window)
#set(myApp_ICON icon.icns)

set(CMAKE_OSX_DEPLOYMENT_TARGET "13.1" CACHE STRING "Minimum OS X deployment version")

addProject("rpg" . ${ENGINE_SRC_DIR})

set_target_properties(
    rpg
    PROPERTIES
    MACOSX_BUNDLE_INFO_PLIST
    ${CMAKE_CURRENT_LIST_DIR}/plist.in
)
