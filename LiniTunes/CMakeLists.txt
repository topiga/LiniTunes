cmake_minimum_required(VERSION 3.5)

project(LiniTunes LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 COMPONENTS Core Quick REQUIRED)

add_executable(LiniTunes
    idevice.cpp
    idevicewatcher.cpp
    main.cpp
    ressource.qrc
)

target_link_libraries(LiniTunes PRIVATE Qt6::Core Qt6::Quick)

target_include_directories(LiniTunes PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

install(TARGETS LiniTunes
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(UNIX AND NOT APPLE)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(LIBIMOBILEDEVICE REQUIRED libimobiledevice-1.0)
    pkg_check_modules(LIBPLIST REQUIRED libplist-2.0)
    pkg_check_modules(LIBUSBMUXD REQUIRED libusbmuxd-2.0)

    target_link_libraries(LiniTunes PRIVATE
        ${LIBIMOBILEDEVICE_LIBRARIES}
        ${LIBPLIST_LIBRARIES}
        ${LIBUSBMUXD_LIBRARIES}
    )

    install(FILES ../linux/linitunes.desktop DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications)
    install(FILES images/linitunes.png DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pixmaps)
endif()
