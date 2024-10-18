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

# Adding ENet
FetchContent_Declare(
  enet
  GIT_REPOSITORY https://github.com/lsalzman/enet.git
  GIT_TAG v1.3.17 # You can change this to the desired version
)

FetchContent_MakeAvailable(enet)

# Adding JSON
FetchContent_Declare(
    json
    URL
    https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
)

FetchContent_MakeAvailable(json)

# Config CEngine
file(GLOB CENGINE_SOURCES "${CMAKE_CURRENT_LIST_DIR}/src/cengine/*.cpp")
add_library(cengine ${CENGINE_SOURCES})
target_link_libraries(cengine raylib enet nlohmann_json::nlohmann_json)
target_include_directories(cengine PUBLIC ${enet_SOURCE_DIR}/include)

# Declaring our executable
if (APPLE)
    # Icon
    set(MACOSX_BUNDLE_ICON_FILE spc)

    set(APPLE_ICON ${CMAKE_CURRENT_SOURCE_DIR}/assets/spc.icns)
    set_source_files_properties(${APPLE_ICON} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")

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

    # Game map
    file(GLOB_RECURSE game_map "${CMAKE_SOURCE_DIR}/assets/map/*")
    foreach (FILE ${game_map})
        file(RELATIVE_PATH NEW_FILE "${CMAKE_SOURCE_DIR}/assets/map" ${FILE})
        get_filename_component(NEW_FILE_PATH ${NEW_FILE} DIRECTORY)
        set_source_files_properties(${FILE}
            PROPERTIES
                MACOSX_PACKAGE_LOCATION "Resources/map/${NEW_FILE_PATH}"
        )
    endforeach()

    # Executable
    add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${APPLE_ICON} ${game_audio} ${game_map} src/main.cpp)

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
target_link_libraries(${PROJECT_NAME} cengine)
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_INCLUDE})

# Checks if OSX and links appropriate frameworks (only required on MacOS)
if (APPLE)
    target_link_libraries(${PROJECT_NAME} "-framework IOKit")
    target_link_libraries(${PROJECT_NAME} "-framework Cocoa")
    target_link_libraries(${PROJECT_NAME} "-framework OpenGL")
endif()
