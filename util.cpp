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

#include <QtCore>
#include <QtGui>
#include <QGuiApplication>
#include <QQuickView>
#include <QDebug>

#include "terminal.h"
#include "util.h"
#include "textrender.h"
#include "version.h"

#ifdef HAVE_FEEDBACK
#include <QFeedbackEffect>
#endif

Util::Util(QSettings *settings, QObject *parent) :
    QObject(parent),
    iAllowGestures(false),
    iSettings(settings),
    iWindow(0),
    iTerm(0)
{
    connect(QGuiApplication::clipboard(), SIGNAL(dataChanged()), this, SIGNAL(clipboardOrSelectionChanged()));
}

Util::~Util()
{
}

void Util::setWindow(QQuickView* win)
{
    if (iWindow)
        qFatal("Should set window property only once");
    iWindow = win;
    if(!iWindow)
        qFatal("invalid main window");
    connect(win, SIGNAL(contentOrientationChanged(Qt::ScreenOrientation)), this, SIGNAL(windowOrientationChanged()));
}

void Util::setWindowTitle(QString title)
{
    iCurrentWinTitle = title;
    iWindow->setTitle(title);
    emit windowTitleChanged();
}

QString Util::windowTitle()
{
    return iCurrentWinTitle;
}

int Util::windowOrientation()
{
    return iWindow->contentOrientation();
}

void Util::setWindowOrientation(int orientation)
{
    iWindow->reportContentOrientationChange(static_cast<Qt::ScreenOrientation>(orientation));
}

void Util::setTerm(Terminal *term)
{
    if (iTerm) {
        qFatal("Should set terminal only once");
    }
    iTerm = term;
    connect(iTerm, SIGNAL(selectionFinished()), this, SIGNAL(clipboardOrSelectionChanged()));
}

void Util::openNewWindow()
{
    QProcess::startDetached("/usr/bin/fingerterm");
}

QString Util::configPath()
{
    if(!iSettings)
        return QString();

    QFileInfo f(iSettings->fileName());
    return f.path();
}

QVariant Util::settingsValue(QString key)
{
    if(!iSettings)
        return QVariant();

    return iSettings->value(key);
}

void Util::setSettingsValue(QString key, QVariant value)
{
    if(iSettings)
        iSettings->setValue(key, value);
}

QString Util::versionString()
{
    return PROGRAM_VERSION;
}

int Util::uiFontSize()
{
    return 12;
}

void Util::keyPressFeedback()
{
    if( !settingsValue("ui/keyPressFeedback").toBool() )
        return;

#ifdef HAVE_FEEDBACK
    QFeedbackEffect::playThemeEffect(QFeedbackEffect::PressWeak);
#endif
}

void Util::keyReleaseFeedback()
{
    if( !settingsValue("ui/keyPressFeedback").toBool() )
        return;

    // TODO: check what's more comfortable, only press, or press and release
#ifdef HAVE_FEEDBACK
    QFeedbackEffect::playThemeEffect(QFeedbackEffect::ReleaseWeak);
#endif
}

void Util::bellAlert()
{
    if(!iWindow)
        return;

    if( settingsValue("general/visualBell").toBool() ) {
        emit visualBell();
    }
}

void Util::notifyText(QString text)
{
    emit notify(text);
}

void Util::copyTextToClipboard(QString str)
{
    QClipboard *cb = QGuiApplication::clipboard();

    cb->clear();
    cb->setText(str);
}

bool Util::terminalHasSelection()
{
    if (!iTerm) {
        return false;
    }
    return !iTerm->selection().isNull();
}

bool Util::canPaste()
{
    QClipboard *cb = QGuiApplication::clipboard();

    return !cb->text().isEmpty();
}

//static
bool Util::charIsHexDigit(QChar ch)
{
    if (ch.isDigit()) // 0-9
        return true;
    else if (ch.toLatin1() >= 65 && ch.toLatin1() <= 70) // A-F
        return true;
    else if (ch.toLatin1() >= 97 && ch.toLatin1() <= 102) // a-f
        return true;

    return false;
}
