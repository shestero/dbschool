cmake_minimum_required(VERSION 3.25.1)
#cmake_minimum_required(VERSION 3.5.1)
# cmake_minimum_required(VERSION 3.31)
project(dbschool1)

set(CMAKE_CXX_STANDARD 14)
#set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_STANDARD 23)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_PREFIX_PATH "/usr/lib/x86_64-linux-gnu/cmake/Qt5")
find_package(Qt5 COMPONENTS
        Core
        Gui
        Widgets
        LinguistTools
        Network
        Concurrent
        REQUIRED)

set(TS_FILES
        ${CMAKE_SOURCE_DIR}/translations/dbschool1_ru.ts
)

set(QT_LUPDATE_EXECUTABLE
        /usr/bin/lupdate
)

# This target will update the .ts files whenever you build it.
add_custom_target(update_translations
        COMMAND ${QT_LUPDATE_EXECUTABLE} ${CMAKE_SOURCE_DIR} -ts ${TS_FILES}
#        COMMAND lupdate ${CMAKE_SOURCE_DIR} -ts ${TS_FILES}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Updating translation files with lupdate"
)

# This ensures that lrelease compiles the .ts files into .qm files during the build process.
#qt5_add_translations(dbschool1 ${TS_FILES})

add_executable(dbschool1
        main.cpp
        mainwindow.cpp
        mainwindow.h
        csvtablemodel.cpp
        csvtablemodel.h
        tableview.cpp
        tableview.h
        ReadOnlyDelegate.h
        ForeignKeyDelegate.cpp
        ForeignKeyDelegate.h
        CustomComboBoxSortFilterProxyModel.cpp
        CustomComboBoxSortFilterProxyModel.h
        Configuration.cpp
        Configuration.h
        LastAttendanceDate.cpp
        LastAttendanceDate.h
        NetworkInteraction.cpp
        NetworkInteraction.h
        ProtocolDialog.cpp
        ProtocolDialog.h
        Worker.h
        generatetables.ui
        generatetables.cpp
        generatetables.h
        generateinvoices.cpp
        generateinvoices.h
        generateinvoices.ui
        Attendance.cpp
        Attendance.h
        ProtocolWidget.cpp
        ProtocolWidget.h
        toolbarcalendarwidget.cpp
        toolbarcalendarwidget.h
)

include(FetchContent)
#FetchContent_Declare(
#        tomlplusplus
#        GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
#        GIT_TAG v3.4.0
#)
#FetchContent_MakeAvailable(tomlplusplus)
#find_package(tomlplusplus CONFIG REQUIRED)
#add_subdirectory(toml++)

FetchContent_Declare(
        toml11
        GIT_REPOSITORY https://github.com/ToruNiina/toml11.git
        GIT_TAG        v4.4.0
)
FetchContent_MakeAvailable(toml11)

FetchContent_Declare(
        QXlsx
        GIT_REPOSITORY https://github.com/QtExcel/QXlsx.git
        GIT_TAG        v1.5.0
        SOURCE_SUBDIR  QXlsx
)
FetchContent_MakeAvailable(QXlsx)

# Путь к исходникам после FetchContent
set(QXLSX_SRC_DIR ${qxlsx_SOURCE_DIR}/QXlsx/source)  # автоматически определяет путь

# Путь к патчу
set(QXLSX_PATCH_FILE ${CMAKE_CURRENT_SOURCE_DIR}/fix-qxlsx-for-mingw.patch)

# Создаём кастомную цель, которая применяется после скачивания, но до сборки
add_custom_target(apply_qxlsx_patch ALL
        COMMAND ${CMAKE_COMMAND} -E echo "Applying QXlsx patch for MinGW..."
        COMMAND echo patch --dry-run -p1 -N -d ${QXLSX_SRC_DIR} FROM ${QXLSX_PATCH_FILE}
        COMMAND patch --dry-run -p1 -N -d ${QXLSX_SRC_DIR} < ${QXLSX_PATCH_FILE} || ${CMAKE_COMMAND} -E echo "Patch already applied, skipping."
        COMMENT "Patching QXlsx for MinGW"
        VERBATIM
)

# Делаем сборку QXlsx зависимой от патча
add_dependencies(QXlsx apply_qxlsx_patch)

target_link_libraries(dbschool1
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        Qt5::Network
        Qt5::Concurrent
        #tomlplusplus::tomlplusplus
        toml11::toml11
        QXlsx::QXlsx
)

target_include_directories(dbschool1 PRIVATE ${qxlsx_SOURCE_DIR}/QXlsx/header)
