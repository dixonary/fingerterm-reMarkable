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
#include <QQuickView>
#include <QDir>
#include <QString>

extern "C" {
#include <pty.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
}

#include "ptyiface.h"
#include "terminal.h"
#include "textrender.h"
#include "util.h"
#include "version.h"
#include "keyloader.h"

#include <QtPlugin>
Q_IMPORT_PLUGIN(QsgEpaperPlugin)

int main(int argc, char *argv[])
{
    qputenv("QMLSCENE_DEVICE", "epaper");
    qputenv("QT_QPA_PLATFORM", "epaper:enable_fonts");
    qputenv("QT_QPA_EVDEV_TOUCHSCREEN_PARAMETERS", "rotate=180");

    QString settings_path(QDir::homePath() + "/.config/FingerTerm");
    QDir dir;

    if (!dir.exists(settings_path)) {
        if (!dir.mkdir(settings_path))
            qWarning() << "Could not create fingerterm settings path" << settings_path;
    }

    QSettings *settings = new QSettings(settings_path + "/settings.ini", QSettings::IniFormat);

    QCoreApplication::setApplicationName("Fingerterm");

    // fork the child process before creating QGuiApplication
    int socketM;
    int pid = forkpty(&socketM,NULL,NULL,NULL);
    if( pid==-1 ) {
        qFatal("forkpty failed");
        exit(1);
    } else if( pid==0 ) {
        setenv("TERM", settings->value("terminal/envVarTERM", "xterm").toByteArray(), 1);

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

    QScreen* sc = app.primaryScreen();
    if(sc){
        sc->setOrientationUpdateMask(Qt::PrimaryOrientation
                                     | Qt::LandscapeOrientation
                                     | Qt::PortraitOrientation
                                     | Qt::InvertedLandscapeOrientation
                                     | Qt::InvertedPortraitOrientation);
    }

    qmlRegisterType<TextRender>("FingerTerm", 1, 0, "TextRender");
    qmlRegisterUncreatableType<Util>("FingerTerm", 1, 0, "Util", "Util is created by app");
    QQuickView view;

    bool fullscreen = !app.arguments().contains("-nofs");
    QSize screenSize = QGuiApplication::primaryScreen()->size();

    if (fullscreen) {
        view.setWidth(screenSize.width());
        view.setHeight(screenSize.height());
    } else {
        view.setWidth(screenSize.width() / 2);
        view.setHeight(screenSize.height() / 2);
    }

    Terminal term;
    Util util(settings); // takes ownership
    term.setUtil(&util);
    TextRender::setUtil(&util);
    TextRender::setTerminal(&term);

    QString startupErrorMsg;

    KeyLoader keyLoader;
    keyLoader.setUtil(&util);
    bool ret = keyLoader.loadLayout(util.keyboardLayout());
    if(!ret) {
        // on failure, try to load the default one (english) directly from resources
        startupErrorMsg = "There was an error loading the keyboard layout.<br>\nUsing the default one instead.";
        util.setKeyboardLayout("english");
        ret = keyLoader.loadLayout(":/data/english.layout");
        if(!ret)
            qFatal("failure loading keyboard layout");
    }

    QQmlContext *context = view.rootContext();
    context->setContextProperty( "term", &term );
    context->setContextProperty( "util", &util );
    context->setContextProperty( "keyLoader", &keyLoader );
    context->setContextProperty( "startupErrorMessage", startupErrorMsg);

    term.setWindow(&view);
    util.setWindow(&view);
    util.setTerm(&term);

    QObject::connect(view.engine(),SIGNAL(quit()),&app,SLOT(quit()));

    view.setResizeMode(QQuickView::SizeRootObjectToView);
    view.engine()->addImportPath(QStringLiteral(DEPLOYMENT_PATH));
    view.setSource(QUrl::fromLocalFile(QStringLiteral(DEPLOYMENT_PATH) + QDir::separator() + QStringLiteral("Main.qml")));

    QObject *root = view.rootObject();
    if(!root)
        qFatal("no root object - qml error");

    if (fullscreen) {
        view.showFullScreen();
    } else {
        view.show();
    }

    PtyIFace ptyiface(pid, socketM, &term, util.charset());

    if( ptyiface.failed() )
        qFatal("pty failure");

    return app.exec();
}
