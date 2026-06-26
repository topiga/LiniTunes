cmake_minimum_required(VERSION 3.16)

project(LiniTunes LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(Qt6 COMPONENTS Core Quick REQUIRED)
find_package(Threads REQUIRED)

# ---- idevice++ integration ------------------------------------------------
set(IDEVICE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/idevice"
    CACHE PATH "Path to idevice repository root")
set(IDEVICE_FFI_PATH "${IDEVICE_ROOT}/target/release/libidevice_ffi.a"
    CACHE FILEPATH "Path to libidevice_ffi.a")
set(IDEVICE_FFI_FEATURES "usbmuxd,diagnostics_relay,pair,afc,installation_proxy"
    CACHE STRING "Cargo features for idevice-ffi build")

# Auto-build Rust FFI static library if missing
if(NOT EXISTS "${IDEVICE_FFI_PATH}")
    message(STATUS "libidevice_ffi.a not found at ${IDEVICE_FFI_PATH}")
    message(STATUS "Building idevice-ffi from source (this may take a minute)...")

    # Ensure submodules are initialized
    if(NOT EXISTS "${IDEVICE_ROOT}/cpp/plist_ffi/cpp/CMakeLists.txt")
        message(STATUS "Initializing idevice submodules...")
        execute_process(
            COMMAND git submodule update --init --recursive --depth 1
            WORKING_DIRECTORY "${IDEVICE_ROOT}"
            RESULT_VARIABLE SUBMODULE_RESULT
        )
        if(NOT SUBMODULE_RESULT EQUAL 0)
            message(FATAL_ERROR "Failed to initialize idevice submodules")
        endif()
    endif()

    # Build the Rust FFI static library
    execute_process(
        COMMAND cargo build --release -p idevice-ffi --features "${IDEVICE_FFI_FEATURES}"
        WORKING_DIRECTORY "${IDEVICE_ROOT}"
        RESULT_VARIABLE CARGO_RESULT
    )
    if(NOT CARGO_RESULT EQUAL 0)
        message(FATAL_ERROR "cargo build failed. Is Rust installed? https://rustup.rs")
    endif()

    if(NOT EXISTS "${IDEVICE_FFI_PATH}")
        message(FATAL_ERROR "cargo build succeeded but ${IDEVICE_FFI_PATH} not found")
    endif()

    message(STATUS "idevice-ffi built successfully: ${IDEVICE_FFI_PATH}")
endif()

# Rebuild target for manual invocation: cmake --build build --target cargo-rebuild
add_custom_target(cargo-rebuild
    COMMAND cargo build --release -p idevice-ffi --features "${IDEVICE_FFI_FEATURES}"
    WORKING_DIRECTORY "${IDEVICE_ROOT}"
    COMMENT "Rebuilding idevice-ffi Rust static library"
)

# Build the C++ wrapper library (idevice++)
add_subdirectory("${IDEVICE_ROOT}/cpp"
                 "${CMAKE_BINARY_DIR}/idevice++"
                 EXCLUDE_FROM_ALL)

# Build plist_ffi C++ library (libPlist++)
set(PLIST_FFI_ROOT "${IDEVICE_ROOT}/cpp/plist_ffi/cpp")
file(GLOB PLIST_CPP_SOURCES "${PLIST_FFI_ROOT}/src/*.cpp")
add_library(plist_ffi_cpp STATIC ${PLIST_CPP_SOURCES})
target_include_directories(plist_ffi_cpp PUBLIC
    "${PLIST_FFI_ROOT}/include"
    "${PLIST_FFI_ROOT}/src"
)
target_link_libraries(plist_ffi_cpp PUBLIC
    idevice++
)

# ---- LiniTunes source files -----------------------------------------------
set(HEADER_FILES
    linitunes_device.h
    idevicewatcher.h
)

set(SOURCE_FILES
    linitunes_device.cpp
    idevicewatcher.cpp
    storage_info.h storage_info.cpp
    storage_sync_worker.h storage_sync_worker.cpp
    main.cpp
)

add_executable(LiniTunes
    ${HEADER_FILES}
    ${SOURCE_FILES}
    ressource.qrc
    thememanager.h thememanager.cpp
    images.qrc
)

target_link_libraries(LiniTunes PRIVATE
    Qt6::Core
    Qt6::Quick
    idevice++
    plist_ffi_cpp
)

target_include_directories(LiniTunes PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

install(TARGETS LiniTunes
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(UNIX AND NOT APPLE)
    install(FILES ../linux/linitunes.desktop DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications)
    install(FILES images/linitunes.png DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pixmaps)
endif()