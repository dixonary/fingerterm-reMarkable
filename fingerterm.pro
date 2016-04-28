QT = core gui qml quick dbus 

CONFIG += link_pkgconfig

TEMPLATE = app
TARGET = fingerterm
DEPENDPATH += .
INCLUDEPATH += .
LIBS += -lutil

# Input
HEADERS += \
    ptyiface.h \
    terminal.h \
    textrender.h \
    version.h \
    util.h \
    keyloader.h \
    mainwindow.h 
SOURCES += \
    main.cpp \
    terminal.cpp \
    textrender.cpp \
    ptyiface.cpp \
    util.cpp \
    keyloader.cpp \
    mainwindow.cpp

OTHER_FILES += \
    qml/Main.qml \
    qml/Keyboard.qml \
    qml/Key.qml \
    qml/Lineview.qml \
    qml/Button.qml \
    qml/MenuFingerterm.qml \
    qml/NotifyWin.qml \
    qml/UrlWindow.qml \
    qml/LayoutWindow.qml

RESOURCES += \
    resources.qrc

unix {
    target.path = /usr/bin
    INSTALLS += target
}

contains(MEEGO_EDITION,nemo) {
    desktopfile.extra = cp $${TARGET}.desktop.nemo $${TARGET}.desktop
    desktopfile.path = /usr/share/applications
    desktopfile.files = $${TARGET}.desktop
    INSTALLS += desktopfile
}
