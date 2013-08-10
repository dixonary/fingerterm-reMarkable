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
class MainWindow;
class TextRender;
class QQuickView;

class Util : public QObject
{
    Q_OBJECT
public:
    explicit Util(QSettings* settings, QObject *parent = 0);
    virtual ~Util();
    void setWindow(QQuickView* win);
    void setWindowTitle(QString title);
    Q_INVOKABLE QString currentWindowTitle();
    void setTerm(Terminal* term) { iTerm = term; }
    void setRenderer(TextRender* r) { iRenderer = r; }

    Q_INVOKABLE void windowMinimize();
    Q_INVOKABLE void openNewWindow();
    Q_INVOKABLE void updateSwipeLock(bool suggestedState);

    Q_INVOKABLE QString versionString();
    Q_INVOKABLE QString configPath();
    Q_INVOKABLE QVariant settingsValue(QString key);
    Q_INVOKABLE void setSettingsValue(QString key, QVariant value);

    Q_INVOKABLE int uiFontSize();
    Q_INVOKABLE bool isHarmattan();

    Q_INVOKABLE void keyPressFeedback();
    Q_INVOKABLE void keyReleaseFeedback();
    Q_INVOKABLE void notifyText(QString text);

    Q_INVOKABLE void copyTextToClipboard(QString str);
    Q_INVOKABLE bool canPaste();
    Q_INVOKABLE bool terminalHasSelection();

    void bellAlert();
    void selectionFinished();

    bool allowGestures() { return iAllowGestures; }
    void setAllowGestures(bool a) { if(iAllowGestures!=a) { iAllowGestures=a; emit allowGesturesChanged(); } }

    Q_PROPERTY(bool allowGestures READ allowGestures WRITE setAllowGestures NOTIFY allowGesturesChanged)

    static bool charIsHexDigit(QChar ch);

public slots:
    void onMainWinFocusChanged(bool in);
    void mousePress(float eventX, float eventY);
    void mouseMove(float eventX, float eventY);
    void mouseRelease(float eventX, float eventY);

signals:
    void visualBell();
    void allowGesturesChanged();
    void gestureNotify(QString msg);
    void clipboardOrSelectionChanged();
    void windowTitleChanged();

private:
    Q_DISABLE_COPY(Util)
    enum PanGesture { PanNone, PanLeft, PanRight, PanUp, PanDown };

    void enableSwipe();
    void disableSwipe();
    bool swipeModeSet;
    bool swipeAllowed;

    void scrollBackBuffer(QPointF now, QPointF last);
    void doGesture(PanGesture gesture);
    void clearNotifications();
    void selectionHelper(QPointF scenePos);

    QPointF dragOrigin;

    bool iAllowGestures;
    bool newSelection;

    QString iCurrentWinTitle;

    QSettings* iSettings;
    MainWindow* iWindow;
    Terminal* iTerm;
    TextRender* iRenderer;
};

#endif // UTIL_H
