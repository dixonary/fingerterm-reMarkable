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

#ifndef UTIL_H
#define UTIL_H

#include <QtCore>

class Terminal;
class TextRender;
class QQuickView;

class Util : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool allowGestures READ allowGestures WRITE setAllowGestures NOTIFY allowGesturesChanged)
    Q_PROPERTY(QString windowTitle READ windowTitle NOTIFY windowTitleChanged)
    Q_PROPERTY(int windowOrientation READ windowOrientation WRITE setWindowOrientation NOTIFY windowOrientationChanged)
    Q_PROPERTY(bool canPaste READ canPaste NOTIFY clipboardOrSelectionChanged)
    Q_PROPERTY(bool terminalHasSelection READ terminalHasSelection NOTIFY clipboardOrSelectionChanged)

public:
    explicit Util(QSettings* settings, QObject *parent = 0);
    virtual ~Util();

    void setWindow(QQuickView* win);
    void setWindowTitle(QString title);
    QString windowTitle();
    int windowOrientation();
    void setWindowOrientation(int orientation);
    void setTerm(Terminal* term);

    Q_INVOKABLE void openNewWindow();

    Q_INVOKABLE QString versionString();
    Q_INVOKABLE QString configPath();
    Q_INVOKABLE QVariant settingsValue(QString key);
    Q_INVOKABLE void setSettingsValue(QString key, QVariant value);

    Q_INVOKABLE int uiFontSize();

    Q_INVOKABLE void keyPressFeedback();
    Q_INVOKABLE void keyReleaseFeedback();
    Q_INVOKABLE void notifyText(QString text);

    Q_INVOKABLE void copyTextToClipboard(QString str);

    bool canPaste();
    bool terminalHasSelection();

    void bellAlert();

    bool allowGestures() { return iAllowGestures; }
    void setAllowGestures(bool a) { if(iAllowGestures!=a) { iAllowGestures=a; emit allowGesturesChanged(); } }

    static bool charIsHexDigit(QChar ch);

signals:
    void visualBell();
    void allowGesturesChanged();
    void notify(QString msg);
    void clipboardOrSelectionChanged();
    void windowTitleChanged();
    void windowOrientationChanged();

private:
    Q_DISABLE_COPY(Util)

    bool iAllowGestures;

    QString iCurrentWinTitle;

    QSettings* iSettings;
    QQuickView* iWindow;
    Terminal* iTerm;
};

#endif // UTIL_H
