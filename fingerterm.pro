QT = core gui qml quick

CONFIG += link_pkgconfig
LIBS += -lqsgepaper
#        LIBS += -L/home/sandsmark/src/qsgepaper/build-qsgepaper-ZG-Release/ -lfreetype -lz #-lbfd
#        TARGETDEPS += /home/sandsmark/src/qsgepaper/build-qsgepaper-ZG-Release/libqsgepaper.a

enable-feedback {
    QT += feedback
    DEFINES += HAVE_FEEDBACK
}

enable-nemonotifications {
    PKGCONFIG += nemonotifications-qt5
}

isEmpty(DEFAULT_FONT) {
    DEFAULT_FONT = NotoMono
}

DEPLOYMENT_PATH = /usr/share/$$TARGET
DEFINES += DEPLOYMENT_PATH=\\\"$$DEPLOYMENT_PATH\\\"
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

qml.files = qml/Main.qml \
    qml/Keyboard.qml \
    qml/Key.qml \
    qml/Lineview.qml \
    qml/Button.qml \
    qml/MenuFingerterm.qml \
    qml/NotifyWin.qml \
    qml/UrlWindow.qml \
    qml/LayoutWindow.qml \
    qml/PopupWindow.qml
qml.path = $$DEPLOYMENT_PATH
INSTALLS += qml

RESOURCES += \
    resources.qrc

icons.files = icons/backspace.png \
    icons/down.png \
    icons/enter.png \
    icons/left.png \
    icons/menu.png \
    icons/right.png \
    icons/scroll-indicator.png \
    icons/shift.png \
    icons/tab.png \
    icons/up.png
icons.path = $$DEPLOYMENT_PATH/icons
INSTALLS += icons

userdata.files = data/menu.xml \
    data/english.layout \
    data/finnish.layout \
    data/french.layout \
    data/german.layout \
    data/qwertz.layout
userdata.path = $$DEPLOYMENT_PATH/data
INSTALLS += userdata

target.path = /usr/bin
INSTALLS += target

contains(MEEGO_EDITION,nemo) {
    desktopfile.extra = cp $${TARGET}.desktop.nemo $${TARGET}.desktop
    desktopfile.path = /usr/share/applications
    desktopfile.files = $${TARGET}.desktop
    INSTALLS += desktopfile
}
