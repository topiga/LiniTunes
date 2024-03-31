cmake_minimum_required(VERSION 3.16)
project(LiniTunes VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(QT NAMES Qt5 Qt6 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Gui Quick QuickControls2)

qt_standard_project_setup()

qt_add_executable(LiniTunes WIN32 MACOSX_BUNDLE
    idevice.cpp idevice.h
    idevicewatcher.cpp idevicewatcher.h
    main.cpp
)
target_link_libraries(LiniTunes PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Quick
    Qt::QuickControls2
)


# Resources:
set(qmake_resources_resource_files
    "main.qml"
)

qt_add_resources(LiniTunes "qmake_resources"
    PREFIX
        "/$${TARGET}"
    FILES
        ${qmake_resources_resource_files}
)
set(resources_resource_files
    "Stroke.qml"
    "files.qml"
    "general.qml"
    "idevices.qml"
    "images/Devices/AppleTV2,1.png"
    "images/Devices/AppleTV3,1.png"
    "images/Devices/AppleTV3,2.png"
    "images/Devices/AppleTV5,3.png"
    "images/Devices/AppleTV6,2.png"
    "images/Devices/AudioAccessory1,1.png"
    "images/Devices/AudioAccessory1,2.png"
    "images/Devices/Generic.png"
    "images/Devices/Watch1,1.png"
    "images/Devices/Watch1,2.png"
    "images/Devices/Watch2,3.png"
    "images/Devices/Watch2,4.png"
    "images/Devices/Watch2,6.png"
    "images/Devices/Watch2,7.png"
    "images/Devices/Watch3,1.png"
    "images/Devices/Watch3,2.png"
    "images/Devices/Watch3,3.png"
    "images/Devices/Watch3,4.png"
    "images/Devices/Watch4,1.png"
    "images/Devices/Watch4,2.png"
    "images/Devices/Watch4,3.png"
    "images/Devices/Watch4,4.png"
    "images/Devices/Watch5,1.png"
    "images/Devices/Watch5,2.png"
    "images/Devices/Watch5,3.png"
    "images/Devices/Watch5,4.png"
    "images/Devices/WatchGeneric.png"
    "images/Devices/iPad1,1.png"
    "images/Devices/iPad11,1.png"
    "images/Devices/iPad11,2.png"
    "images/Devices/iPad11,3.png"
    "images/Devices/iPad11,4.png"
    "images/Devices/iPad11,6.png"
    "images/Devices/iPad11,7.png"
    "images/Devices/iPad13,1.png"
    "images/Devices/iPad13,2.png"
    "images/Devices/iPad2,1.png"
    "images/Devices/iPad2,2.png"
    "images/Devices/iPad2,3.png"
    "images/Devices/iPad2,4.png"
    "images/Devices/iPad2,5.png"
    "images/Devices/iPad2,6.png"
    "images/Devices/iPad2,7.png"
    "images/Devices/iPad3,1.png"
    "images/Devices/iPad3,2.png"
    "images/Devices/iPad3,3.png"
    "images/Devices/iPad3,4.png"
    "images/Devices/iPad3,5.png"
    "images/Devices/iPad3,6.png"
    "images/Devices/iPad4,1.png"
    "images/Devices/iPad4,2.png"
    "images/Devices/iPad4,3.png"
    "images/Devices/iPad4,4.png"
    "images/Devices/iPad4,5.png"
    "images/Devices/iPad4,6.png"
    "images/Devices/iPad4,7.png"
    "images/Devices/iPad4,8.png"
    "images/Devices/iPad4,9.png"
    "images/Devices/iPad5,1.png"
    "images/Devices/iPad5,2.png"
    "images/Devices/iPad5,3.png"
    "images/Devices/iPad5,4.png"
    "images/Devices/iPad6,11.png"
    "images/Devices/iPad6,12.png"
    "images/Devices/iPad6,3.png"
    "images/Devices/iPad6,4.png"
    "images/Devices/iPad6,7.png"
    "images/Devices/iPad6,8.png"
    "images/Devices/iPad7,1.png"
    "images/Devices/iPad7,11.png"
    "images/Devices/iPad7,12.png"
    "images/Devices/iPad7,2.png"
    "images/Devices/iPad7,3.png"
    "images/Devices/iPad7,4.png"
    "images/Devices/iPad7,5.png"
    "images/Devices/iPad7,6.png"
    "images/Devices/iPad8,1.png"
    "images/Devices/iPad8,10.png"
    "images/Devices/iPad8,11.png"
    "images/Devices/iPad8,12.png"
    "images/Devices/iPad8,2.png"
    "images/Devices/iPad8,3.png"
    "images/Devices/iPad8,4.png"
    "images/Devices/iPad8,5.png"
    "images/Devices/iPad8,6.png"
    "images/Devices/iPad8,7.png"
    "images/Devices/iPad8,8.png"
    "images/Devices/iPad8,9.png"
    "images/Devices/iPadGeneric.png"
    "images/Devices/iPhone1,1.png"
    "images/Devices/iPhone1,2.png"
    "images/Devices/iPhone10,1.png"
    "images/Devices/iPhone10,2.png"
    "images/Devices/iPhone10,3.png"
    "images/Devices/iPhone10,4.png"
    "images/Devices/iPhone10,5.png"
    "images/Devices/iPhone10,6.png"
    "images/Devices/iPhone11,2.png"
    "images/Devices/iPhone11,4.png"
    "images/Devices/iPhone11,6.png"
    "images/Devices/iPhone11,8.png"
    "images/Devices/iPhone12,1.png"
    "images/Devices/iPhone12,3.png"
    "images/Devices/iPhone12,5.png"
    "images/Devices/iPhone12,8.png"
    "images/Devices/iPhone13,1.png"
    "images/Devices/iPhone13,2.png"
    "images/Devices/iPhone13,3.png"
    "images/Devices/iPhone13,4.png"
    "images/Devices/iPhone14,2.png"
    "images/Devices/iPhone14,3.png"
    "images/Devices/iPhone14,4.png"
    "images/Devices/iPhone14,5.png"
    "images/Devices/iPhone14,6.png"
    "images/Devices/iPhone14,7.png"
    "images/Devices/iPhone14,8.png"
    "images/Devices/iPhone15,2.png"
    "images/Devices/iPhone15,3.png"
    "images/Devices/iPhone2,1.png"
    "images/Devices/iPhone3,1.png"
    "images/Devices/iPhone3,2.png"
    "images/Devices/iPhone3,3.png"
    "images/Devices/iPhone4,1.png"
    "images/Devices/iPhone5,1.png"
    "images/Devices/iPhone5,2.png"
    "images/Devices/iPhone5,3.png"
    "images/Devices/iPhone5,4.png"
    "images/Devices/iPhone6,1.png"
    "images/Devices/iPhone6,2.png"
    "images/Devices/iPhone7,1.png"
    "images/Devices/iPhone7,2.png"
    "images/Devices/iPhone8,1.png"
    "images/Devices/iPhone8,2.png"
    "images/Devices/iPhone8,4.png"
    "images/Devices/iPhone9,1.png"
    "images/Devices/iPhone9,2.png"
    "images/Devices/iPhone9,3.png"
    "images/Devices/iPhone9,4.png"
    "images/Devices/iPhoneGeneric.png"
    "images/Devices/iPod1,1.png"
    "images/Devices/iPod2,1.png"
    "images/Devices/iPod3,1.png"
    "images/Devices/iPod4,1.png"
    "images/Devices/iPod5,1.png"
    "images/Devices/iPod7,1.png"
    "images/Devices/iPod9,1.png"
    "images/Files/Files_black_30x30.png"
    "images/Files/Files_dark_30x30.png"
    "images/Files/Files_light_30x30.png"
    "images/Files/Files_white_30x30.png"
    "images/General/General_black_30x30.png"
    "images/General/General_dark_30x30.png"
    "images/General/General_light_30x30.png"
    "images/General/General_white_30x30.png"
    "images/iDevice/iDevice_black_90x90.png"
    "images/iDevice/iDevice_dark_90x90.png"
    "images/iDevice/iDevice_light_90x90.png"
    "images/iDevice/iDevice_white_90x90.png"
    "images/linitunes.png"
    "test.qml"
)

qt_add_resources(LiniTunes "resources"
    PREFIX
        "/"
    FILES
        ${resources_resource_files}
)

if(UNIX AND NOT MACOS)
    target_link_libraries(LiniTunes PRIVATE
        imobiledevice-1.0
        plist-2.0
        usbmuxd-2.0
    )
endif()

install(TARGETS LiniTunes
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt_generate_deploy_app_script(
    TARGET LiniTunes
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
