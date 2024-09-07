cmake_minimum_required(VERSION 3.15)

project(spc CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PROJECT_VERSION "0.1.0")

# Adding Raylib
include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE) # don't build the supplied example games

set(RAYLIB_VERSION 5.0)
FetchContent_Declare(
    raylib
    URL https://github.com/raysan5/raylib/archive/refs/tags/${RAYLIB_VERSION}.tar.gz
    FIND_PACKAGE_ARGS ${RAYLIB_VERSION}
)

FetchContent_MakeAvailable(raylib)

# Adding Raygui
set(RAYGUI_VERSION 4.0)
FetchContent_Declare(
    raygui
    URL https://github.com/raysan5/raygui/archive/refs/tags/${RAYGUI_VERSION}.tar.gz
    FIND_PACKAGE_ARGS ${RAYGUI_VERSION}
)

FetchContent_MakeAvailable(raygui)

# Config CEngine
file(GLOB CENGINE_SOURCES "${CMAKE_CURRENT_LIST_DIR}/src/cengine/*.cpp")
add_library(cengine ${CENGINE_SOURCES})
target_include_directories(cengine PRIVATE ${CMAKE_BINARY_DIR}/_deps/raygui-src/src/)
target_link_libraries(cengine raylib)

# Declaring our executable
if (APPLE)
    # Game audio
    file(GLOB_RECURSE game_audio "${CMAKE_SOURCE_DIR}/assets/audio/*.wav")
    foreach (FILE ${game_audio})
        file(RELATIVE_PATH NEW_FILE "${CMAKE_SOURCE_DIR}/assets/audio" ${FILE})
        get_filename_component(NEW_FILE_PATH ${NEW_FILE} DIRECTORY)
        set_source_files_properties(${FILE}
            PROPERTIES
                MACOSX_PACKAGE_LOCATION "Resources/audio/${NEW_FILE_PATH}"
        )
    endforeach()

    # Icon
    set(MACOSX_BUNDLE_ICON_FILE spc.icns)
    set(APPLE_ICON ${CMAKE_CURRENT_SOURCE_DIR}/assets/spc.icns)
    set_source_files_properties(${APPLE_ICON} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
    
    # Executable
    add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${APPLE_ICON} ${game_audio} src/main.cpp)

    # Adding our source files
    file(GLOB PROJECT_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp")
    set(PROJECT_INCLUDE "${CMAKE_CURRENT_LIST_DIR}/include/")

    # Linking CoreFoundation
    find_library(COREFOUNDATION_FRAMEWORK CoreFoundation)
    if(COREFOUNDATION_FRAMEWORK)
        target_link_libraries(${PROJECT_NAME} ${COREFOUNDATION_FRAMEWORK})
    else()
        message(FATAL_ERROR "CoreFoundation framework not found")
    endif()

    # Set bundle properties
    set_target_properties(${CMAKE_PROJECT_NAME}
        PROPERTIES # https://cmake.org/cmake/help/latest/prop_tgt/MACOSX_BUNDLE_INFO_PLIST.html
            MACOSX_BUNDLE_BUNDLE_NAME "${CMAKE_PROJECT_NAME}" # CFBundleIdentifier
            MACOSX_BUNDLE_GUI_IDENTIFIER "com.our-company"
            MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION} # CFBundleLongVersionString, deprecated
            MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION} # CFBundleShortVersionString
    )
else()
    # Setting ASSETS_PATH
    # target_compile_definitions(${PROJECT_NAME} PUBLIC ASSETS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/assets/") # Set the asset path macro to the absolute path on the dev machine
    # target_compile_definitions(${PROJECT_NAME} PUBLIC ASSETS_PATH="./assets") # Set the asset path macro in release mode to a relative path that assumes the assets folder is in the same directory as the game executable
    
    add_executable(${PROJECT_NAME} src/main.cpp)
endif()

target_sources(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_INCLUDE})
# target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_BINARY_DIR}/_deps/raygui-src/src/)
target_link_libraries(${PROJECT_NAME} cengine)

# Checks if OSX and links appropriate frameworks (only required on MacOS)
if (APPLE)
    target_link_libraries(${PROJECT_NAME} "-framework IOKit")
    target_link_libraries(${PROJECT_NAME} "-framework Cocoa")
    target_link_libraries(${PROJECT_NAME} "-framework OpenGL")
endif()
