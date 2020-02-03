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
    iSettings(settings),
    iWindow(0),
    iTerm(0)
{
    connect(QGuiApplication::clipboard(), SIGNAL(dataChanged()), this, SIGNAL(clipboardOrSelectionChanged()));
}

Util::~Util()
{
    delete iSettings;
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

QVariant Util::settingsValue(QString key, const QVariant &defaultValue)
{
    if(!iSettings)
        return defaultValue;

    return iSettings->value(key, defaultValue);
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

int Util::fontSize()
{
    return settingsValue("ui/fontSize", 11).toInt();
}

void Util::setFontSize(int size)
{
    if (size == fontSize()) {
        return;
    }

    setSettingsValue("ui/fontSize", size);
    emit fontSizeChanged();
}

void Util::keyPressFeedback()
{
    if( !settingsValue("ui/keyPressFeedback", true).toBool() )
        return;

#ifdef HAVE_FEEDBACK
    QFeedbackEffect::playThemeEffect(QFeedbackEffect::PressWeak);
#endif
}

void Util::keyReleaseFeedback()
{
    if( !settingsValue("ui/keyPressFeedback", true).toBool() )
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

    if( settingsValue("general/visualBell", true).toBool() ) {
        emit visualBell();
    }
}

QString Util::fontFamily()
{
    return settingsValue("ui/fontFamily", DEFAULT_FONTFAMILY).toString();
}

int Util::dragMode()
{
    QString mode = settingsValue("ui/dragMode", "scroll").toString();

    if (mode == "gestures") {
        return DragGestures;
    } else if (mode == "scroll") {
        return DragScroll;
    } else if (mode == "select") {
        return DragSelect;
    } else {
        return DragOff;
    }
}

void Util::setDragMode(int mode)
{
    if (mode == dragMode()) {
        return;
    }

    QString modeString;
    switch(mode) {
    case DragGestures:
        modeString = "gestures";
        break;
    case DragScroll:
        modeString = "scroll";
        break;
    case DragSelect:
        modeString = "select";
        break;
    case DragOff:
    default:
        modeString = "off";
    }

    setSettingsValue("ui/dragMode", modeString);
    emit dragModeChanged();
}

int Util::keyboardMode()
{
    QString mode = settingsValue("ui/vkbShowMethod", "move").toString();

    if (mode == "fade") {
        return KeyboardFade;
    } else if (mode == "move") {
        return KeyboardMove;
    } else {
        return KeyboardOff;
    }
}

void Util::setKeyboardMode(int mode)
{
    if (mode == keyboardMode()) {
        return;
    }

    QString modeString;
    switch(mode) {
    case KeyboardFade:
        modeString = "fade";
        break;
    case KeyboardMove:
        modeString = "move";
        break;
    case KeyboardOff:
    default:
        modeString = "off";
    }

    setSettingsValue("ui/vkbShowMethod", modeString);
    emit keyboardModeChanged();
}

int Util::keyboardFadeOutDelay()
{
    return settingsValue("ui/keyboardFadeOutDelay", 4000).toInt();
}

void Util::setKeyboardFadeOutDelay(int delay)
{
    if (delay == keyboardFadeOutDelay()) {
        return;
    }

    setSettingsValue("ui/keyboardFadeOutDelay", delay);
    emit keyboardFadeOutDelayChanged();
}

QString Util::keyboardLayout()
{
    return settingsValue("ui/keyboardLayout", "english").toString();
}

void Util::setKeyboardLayout(const QString &layout)
{
    if (layout == keyboardLayout()) {
        return;
    }

    setSettingsValue("ui/keyboardLayout", layout);
    emit keyboardLayoutChanged();
}

int Util::extraLinesFromCursor()
{
    return settingsValue("ui/showExtraLinesFromCursor", 1).toInt();
}

QString Util::charset()
{
    return settingsValue("terminal/charset", "UTF-8").toString();
}

int Util::keyboardMargins()
{
    return settingsValue("ui/keyboardMargins", 10).toInt();
}

bool Util::showVisualKeyPressFeedback()
{
    return settingsValue("ui/showVisualKeyPressFeedback", true).toBool();
}

int Util::orientationMode()
{
    QString mode = settingsValue("ui/orientationLockMode", "auto").toString();

    if (mode == "auto") {
        return OrientationAuto;
    } else if (mode == "landscape") {
        return OrientationLandscape;
    } else {
        return OrientationPortrait;
    }
}

void Util::setOrientationMode(int mode)
{
    if (mode == orientationMode()) {
        return;
    }

    QString modeString;
    switch(mode) {
    case OrientationAuto:
        modeString = "auto";
        break;
    case OrientationLandscape:
        modeString = "landscape";
        break;
    case OrientationPortrait:
    default:
        modeString = "portrait";
    }

    setSettingsValue("ui/orientationLockMode", modeString);
    emit orientationModeChanged();
}

bool Util::showWelcomeScreen()
{
    return settingsValue("state/showWelcomeScreen", true).toBool();
}

void Util::setShowWelcomeScreen(bool value)
{
    if (value != showWelcomeScreen()) {
        setSettingsValue("state/showWelcomeScreen", value);
        emit showWelcomeScreenChanged();
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
