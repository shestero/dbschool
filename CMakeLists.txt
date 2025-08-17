cmake_minimum_required(VERSION 3.5.1)
#cmake_minimum_required(VERSION 3.25.1)
# cmake_minimum_required(VERSION 3.31)
project(dbschool1)

set(CMAKE_CXX_STANDARD 14)
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


add_executable(dbschool1 main.cpp
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
        CustomComboBoxSortFilterProxyModel.h)

target_link_libraries(dbschool1
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
)

