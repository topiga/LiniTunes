QT += quick svg

SOURCES += \
        idevice.cpp \
        main.cpp

resources.files = main.qml 
resources.prefix = /$${TARGET}
RESOURCES += resources \
    resources.qrc

TRANSLATIONS += \
    LiniTunes_fr_FR.ts
CONFIG += lrelease
CONFIG += embed_translations

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    files.qml \
    general.qml \
    idevices.qml \
    test.qml

HEADERS += \
    idevice.h

unix:!macx {
    LIBS += -limobiledevice-1.0
    LIBS += -lplist-2.0
    isEmpty(PREFIX) {
            PREFIX = /usr/local
    }

    target.path = $$PREFIX/bin

    shortcutfiles.files = ../linux/linitunes.desktop
    shortcutfiles.path = $$PREFIX/share/applications/
    data.files += images/linitunes.png
    data.path = $$PREFIX/share/pixmaps/

    INSTALLS += shortcutfiles
    INSTALLS += data
    INSTALLS += target

    DISTFILES += \
        ../linux/linitunes.desktop \
        images/linitunes.png

}
