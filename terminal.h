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

#ifndef TERMINAL_H
#define TERMINAL_H

#include <QtCore>
#include <QWidget>

class TextRender;
class PtyIFace;
class Util;

struct TermChar {
    QChar c;
    int fgColor;
    int bgColor;
    int attrib;
};

const int attribNone = 0;
const int attribBold = 1;
const int attribUnderline = 2;
const int attribNegative = 4;
const QByteArray multiCharEscapes("().*+-/%#");

struct TermAttribs {
    QPoint cursorPos;

    bool wrapAroundMode;
    bool originMode;

    int currentFgColor;
    int currentBgColor;
    int currentAttrib;
};

class Terminal : public QObject
{
    Q_OBJECT
public:
    static const int defaultFgColor = 7;
    static const int defaultBgColor = 0;

    explicit Terminal(QObject *parent = 0);
    virtual ~Terminal() {}
    void setRenderer(TextRender* tr);
    void setPtyIFace(PtyIFace* pty);
    void setWindow(QWidget* win) { iWindow=win; }
    void setUtil(Util* util) { iUtil = util; }

    void insertInBuffer(const QString& chars);

    QPoint cursorPos();
    void setCursorPos(QPoint pos);
    bool showCursor();

    Q_INVOKABLE QSize termSize() { return iTermSize; }
    void setTermSize(QSize size);

    QList<QList<TermChar> >& buffer();
    QList<QList<TermChar> >& backBuffer() { return iBackBuffer; }

    QList<TermChar>& currentLine();

    Q_INVOKABLE void keyPress(int key, int modifiers);
    Q_INVOKABLE const QStringList printableLinesFromCursor(int lines);
    Q_INVOKABLE void putString(QString str, bool unEscape=false);

    Q_INVOKABLE void pasteFromClipboard();
    Q_INVOKABLE void copySelectionToClipboard();
    Q_INVOKABLE const QStringList grabURLsFromBuffer();

    Q_INVOKABLE QString getUserMenuXml();

    void scrollBackBufferFwd(int lines);
    void scrollBackBufferBack(int lines);
    int backBufferScrollPos() { return iBackBufferScrollPos; }
    void resetBackBufferScrollPos();

    void setSelection(QPoint start, QPoint end);
    QRect selection();
    Q_INVOKABLE void clearSelection();
    bool hasSelection();

    TermChar zeroChar;

signals:
    void cursorPosChanged(QPoint newPos);
    void termSizeChanged(QSize newSize);
    void displayBufferChanged();

private:
    Q_DISABLE_COPY(Terminal)
    static const char ch_ESC = 0x1B; //escape
    static const int maxScrollBackLines = 300;

    void insertAtCursor(QChar c, bool overwriteMode=true, bool advanceCursor=true);
    void deleteAt(QPoint pos);
    void clearAt(QPoint pos);
    void eraseLineAtCursor(int from=-1, int to=-1);
    void clearAll(bool wholeBuffer=false);
    void ansiSequence(const QString& seq);
    void oscSequence(const QString& seq);
    void escControlChar(const QString& seq);
    void trimBackBuffer();
    void scrollBack(int lines, int insertAt=-1);
    void scrollFwd(int lines, int removeAt=-1);
    void resetTerminal();
    void resetTabs();
    void adjustSelectionPosition(int lines);

    TextRender* iRenderer;
    PtyIFace* iPtyIFace;
    QWidget* iWindow;
    Util* iUtil;

    QList<QList<TermChar> > iBuffer;
    QList<QList<TermChar> > iAltBuffer;
    QList<QList<TermChar> > iBackBuffer;
    QList<QList<int> > iTabStops;

    QSize iTermSize;
    bool iEmitCursorChangeSignal;

    bool iShowCursor;
    bool iUseAltScreenBuffer;
    bool iAppCursorKeys;
    bool iReplaceMode;
    bool iNewLineMode;

    int iMarginTop;
    int iMarginBottom;

    int iBackBufferScrollPos;

    TermAttribs iTermAttribs;
    TermAttribs iTermAttribs_saved;
    TermAttribs iTermAttribs_saved_alt;

    QString escSeq;
    QString oscSeq;
    int escape;
    QRect iSelection;
};

#endif // TERMINAL_H
