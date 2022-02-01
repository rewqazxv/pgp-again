cmake_minimum_required(VERSION 3.5)

project(pgp-again LANGUAGES CXX)

if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/conanbuildinfo.cmake)
    include(conanbuildinfo.cmake)
    conan_basic_setup()
endif()

add_subdirectory(lib/cpp-crypto)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

if(NOT QT_VERSION)
    set(QT_VERSION 5)
endif()
find_package(Qt${QT_VERSION} COMPONENTS Widgets LinguistTools REQUIRED)

set(TS_FILES src/lang/zh_CN.ts)
add_custom_target(pgp-again_translations ALL
DEPENDS
    ${TS_FILES}
)
qt_create_translation(_ src ${TS_FILES})

add_executable(pgp-again
    src/main.cpp
    src/config.h
    src/ui/main-window.cpp
    src/ui/main-window.h
    src/ui/main-window.ui
    src/ui/key-import-dialog.cpp
    src/ui/key-import-dialog.h
    src/ui/key-import-dialog.ui
    src/ui/generate-key-pair-dialog.cpp
    src/ui/generate-key-pair-dialog.h
    src/ui/generate-key-pair-dialog.ui
    src/ui/progress-dialog.cpp
    src/ui/progress-dialog.h
    src/ui/progress-dialog.ui
    src/res.qrc
    src/model/keys-model.h
    src/model/key-info.h
    src/core/pgp-task.h
    src/core/pgp-task.cpp
)
target_include_directories(pgp-again PRIVATE src)
target_precompile_headers(pgp-again PRIVATE src/pch.h)
target_link_libraries(pgp-again
PRIVATE
    Qt::Widgets
    crypto
)
if(WIN32)
    set_target_properties(pgp-again PROPERTIES WIN32_EXECUTABLE true)
endif()
