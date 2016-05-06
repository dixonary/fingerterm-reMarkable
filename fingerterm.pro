QT = core gui qml quick

CONFIG += link_pkgconfig

enable-feedback {
    QT += feedback
    DEFINES += HAVE_FEEDBACK
}

enable-nemonotifications {
    PKGCONFIG += nemonotifications-qt5
}

isEmpty(DEFAULT_FONT) {
    DEFAULT_FONT = monospace
}

DEFINES += DEFAULT_FONTFAMILY=\\\"$$DEFAULT_FONT\\\"

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
    keyloader.h

SOURCES += \
    main.cpp \
    terminal.cpp \
    textrender.cpp \
    ptyiface.cpp \
    util.cpp \
    keyloader.cpp

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

target.path = /usr/bin
INSTALLS += target

contains(MEEGO_EDITION,nemo) {
    desktopfile.extra = cp $${TARGET}.desktop.nemo $${TARGET}.desktop
    desktopfile.path = /usr/share/applications
    desktopfile.files = $${TARGET}.desktop
    INSTALLS += desktopfile
}
