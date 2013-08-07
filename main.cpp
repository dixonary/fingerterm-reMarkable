/*
    Copyright 2011-2012 Heikki Holstila <heikki.holstila@gmail.com>

    This file is part of FingerTerm.

    FingerTerm is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    FingerTerm is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with FingerTerm.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "qplatformdefs.h"

#include <QtGui>
#include <QtQml>

extern "C" {
#include <pty.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
}

#ifdef MEEGO_EDITION_HARMATTAN
#include <MComponentData>
#include "dbusadaptor.h"
#endif

#include "mainwindow.h"
#include "ptyiface.h"
#include "terminal.h"
#include "textrender.h"
#include "util.h"
#include "version.h"
#include "keyloader.h"

void defaultSettings(QSettings* settings);
void copyFileFromResources(QString from, QString to);

int main(int argc, char *argv[])
{
    QSettings *settings = new QSettings(QDir::homePath()+"/.config/FingerTerm/settings.ini", QSettings::IniFormat);
    defaultSettings(settings);

    // fork the child process before creating QGuiApplication
    int socketM;
    int pid = forkpty(&socketM,NULL,NULL,NULL);
    if( pid==-1 ) {
        qFatal("forkpty failed");
        exit(1);
    } else if( pid==0 ) {
        setenv("TERM", settings->value("terminal/envVarTERM").toByteArray(), 1);

        QString execCmd;
        for(int i=0; i<argc-1; i++) {
            if( QString(argv[i]) == "-e" )
                execCmd = QString(argv[i+1]);
        }
        if(execCmd.isEmpty()) {
            execCmd = settings->value("general/execCmd").toString();
        }
        if(execCmd.isEmpty()) {
            // execute the user's default shell
            passwd *pwdstruct = getpwuid(getuid());
            execCmd = QString(pwdstruct->pw_shell);
            execCmd.append(" --login");
        }

        if(settings)
            delete settings; // don't need 'em here

        QStringList execParts = execCmd.split(' ', QString::SkipEmptyParts);
        if(execParts.length()==0)
            exit(0);
        char *ptrs[execParts.length()+1];
        for(int i=0; i<execParts.length(); i++) {
            ptrs[i] = new char[execParts.at(i).toLatin1().length()+1];
            memcpy(ptrs[i], execParts.at(i).toLatin1().data(), execParts.at(i).toLatin1().length());
            ptrs[i][execParts.at(i).toLatin1().length()] = 0;
        }
        ptrs[execParts.length()] = 0;

        execvp(execParts.first().toLatin1(), ptrs);
        exit(0);
    }

    QGuiApplication app(argc, argv);

    qmlRegisterType<TextRender>("TextRender",1,0,"TextRender");
    MainWindow view;

#ifdef MEEGO_EDITION_HARMATTAN
    DbusAdaptor *dba = new DbusAdaptor();
    dba->setAppWindow(&view);

    // needed for MFeedback, also creates the dbus interface
    MComponentData::createInstance(argc, argv, "fingerterm", dba);
#endif

    Terminal term;
    Util util(settings);
    term.setUtil(&util);
    QString startupErrorMsg;

    // copy the default config files to the config dir if they don't already exist
    copyFileFromResources(":/data/menu.xml", util.configPath()+"/menu.xml");
    copyFileFromResources(":/data/english.layout", util.configPath()+"/english.layout");
    copyFileFromResources(":/data/finnish.layout", util.configPath()+"/finnish.layout");

    KeyLoader keyLoader;
    keyLoader.setUtil(&util);
    bool ret = keyLoader.loadLayout( settings->value("ui/keyboardLayout").toString() );
    if(!ret) {
        // on failure, try to load the default one (english) directly from resources
        startupErrorMsg = "There was an error loading the keyboard layout.<br>\nUsing the default one instead.";
        settings->setValue("ui/keyboardLayout", "english");
        ret = keyLoader.loadLayout(":/data/english.layout");
        if(!ret)
            qFatal("failure loading keyboard layout");
    }

    QQmlContext *context = view.rootContext();
    context->setContextProperty( "term", &term );
    context->setContextProperty( "util", &util );
    context->setContextProperty( "keyLoader", &keyLoader );

    view.setSource(QUrl("qrc:/qml/Main.qml"));

    QObject *win = view.rootObject();
    if(!win)
        qFatal("no root object - qml error");

    if(!startupErrorMsg.isEmpty())
        QMetaObject::invokeMethod(win, "showErrorMessage", Qt::QueuedConnection, Q_ARG(QVariant, startupErrorMsg));

    TextRender *tr = win->findChild<TextRender*>("textrender");
    tr->setUtil(&util);
    tr->setTerminal(&term);
    term.setRenderer(tr);
    term.setWindow(&view);
    util.setWindow(&view);
    util.setTerm(&term);
    util.setRenderer(tr);
    view.installEventFilter(&util); //for grabbing mouse drags

    QObject::connect(&term,SIGNAL(displayBufferChanged()),win,SLOT(displayBufferChanged()));
    QObject::connect(view.engine(),SIGNAL(quit()),&app,SLOT(quit()));

#ifdef MEEGO_EDITION_HARMATTAN
    view.showFullScreen();
#else
    QSize screenSize = QGuiApplication::primaryScreen()->size();
    if ((screenSize.width() < 1024 || screenSize.height() < 768 || app.arguments().contains("-fs"))
            && !app.arguments().contains("-nofs"))
    {
        view.showFullScreen();
    } else
        view.show();
#endif

    PtyIFace ptyiface(pid, socketM, &term,
                       settings->value("terminal/charset").toString());

    if( ptyiface.failed() )
        qFatal("pty failure");

    return app.exec();
}

void defaultSettings(QSettings* settings)
{
    if(!settings->contains("general/execCmd"))
        settings->setValue("general/execCmd", "");
    if(!settings->contains("general/visualBell"))
        settings->setValue("general/visualBell", true);
    if(!settings->contains("general/backgroundBellNotify"))
        settings->setValue("general/backgroundBellNotify", true);
    if(!settings->contains("general/grabUrlsFromBackbuffer"))
        settings->setValue("general/grabUrlsFromBackbuffer", false);

    if(!settings->contains("terminal/envVarTERM"))
        settings->setValue("terminal/envVarTERM", "xterm");
    if(!settings->contains("terminal/charset"))
        settings->setValue("terminal/charset", "UTF-8");

    if(!settings->contains("ui/keyboardLayout"))
        settings->setValue("ui/keyboardLayout", "english");
    if(!settings->contains("ui/fontFamily"))
        settings->setValue("ui/fontFamily", "monospace");
    if(!settings->contains("ui/fontSize"))
        settings->setValue("ui/fontSize", 11);
    if(!settings->contains("ui/keyboardMargins"))
        settings->setValue("ui/keyboardMargins", 10);
    if(!settings->contains("ui/allowSwipe"))
        settings->setValue("ui/allowSwipe", "auto");   // "true", "false", "auto"
    if(!settings->contains("ui/keyboardFadeOutDelay"))
        settings->setValue("ui/keyboardFadeOutDelay", 2500);
    if(!settings->contains("ui/showExtraLinesFromCursor"))
        settings->setValue("ui/showExtraLinesFromCursor", 1);
    if(!settings->contains("ui/vkbShowMethod"))
        settings->setValue("ui/vkbShowMethod", "fade");  // "fade", "move", "off"
    if(!settings->contains("ui/keyPressFeedback"))
        settings->setValue("ui/keyPressFeedback", true);
    if(!settings->contains("ui/dragMode"))
        settings->setValue("ui/dragMode", "gestures");  // "gestures, "scroll", "select" ("off" would also be ok)

    if(!settings->contains("state/showWelcomeScreen"))
        settings->setValue("state/showWelcomeScreen", true);
    if(!settings->contains("state/createdByVersion"))
        settings->setValue("state/createdByVersion", PROGRAM_VERSION);

    if(!settings->contains("gestures/panLeftTitle"))
        settings->setValue("gestures/panLeftTitle", "Alt-Right");
    if(!settings->contains("gestures/panLeftCommand"))
        settings->setValue("gestures/panLeftCommand", "\\e\\e[C");
    if(!settings->contains("gestures/panRightTitle"))
        settings->setValue("gestures/panRightTitle", "Alt-Left");
    if(!settings->contains("gestures/panRightCommand"))
        settings->setValue("gestures/panRightCommand", "\\e\\e[D");
    if(!settings->contains("gestures/panUpTitle"))
        settings->setValue("gestures/panUpTitle", "Page Down");
    if(!settings->contains("gestures/panUpCommand"))
        settings->setValue("gestures/panUpCommand", "\\e[6~");
    if(!settings->contains("gestures/panDownTitle"))
        settings->setValue("gestures/panDownTitle", "Page Up");
    if(!settings->contains("gestures/panDownCommand"))
        settings->setValue("gestures/panDownCommand", "\\e[5~");
}

void copyFileFromResources(QString from, QString to)
{
    // copy a file from resources to the config dir if it does not exist there
    QFileInfo toFile(to);
    if(!toFile.exists()) {
        QFile newToFile(toFile.absoluteFilePath());
        QResource res(from);
        if (newToFile.open(QIODevice::WriteOnly)) {
            newToFile.write( reinterpret_cast<const char*>(res.data()) );
            newToFile.close();
        } else {
            qFatal("failed to copy default config from resources");
        }
    }
}
