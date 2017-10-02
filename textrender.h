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

#ifndef TEXTRENDER_H
#define TEXTRENDER_H

#include <QQuickPaintedItem>
#include <QPainter>

#include "terminal.h"

class Util;

class TextRender : public QQuickPaintedItem
{
    Q_PROPERTY(int fontWidth READ fontWidth NOTIFY fontSizeChanged)
    Q_PROPERTY(int fontHeight READ fontHeight NOTIFY fontSizeChanged)
    Q_PROPERTY(int fontPointSize READ fontPointSize WRITE setFontPointSize NOTIFY fontSizeChanged)
    Q_PROPERTY(bool showBufferScrollIndicator READ showBufferScrollIndicator WRITE setShowBufferScrollIndicator NOTIFY showBufferScrollIndicatorChanged)
    Q_PROPERTY(bool allowGestures READ allowGestures WRITE setAllowGestures NOTIFY allowGesturesChanged)

    Q_OBJECT
public:
    explicit TextRender(QQuickItem *parent = 0);
    virtual ~TextRender();
    void paint(QPainter*);

    static void setUtil(Util *util);
    static void setTerminal(Terminal *terminal);

    int fontWidth() { return iFontWidth; }
    int fontHeight() { return iFontHeight; }
    int fontDescent() { return iFontDescent; }
    int fontPointSize() { return iFont.pointSize(); }
    void setFontPointSize(int psize);
    bool showBufferScrollIndicator() { return iShowBufferScrollIndicator; }
    void setShowBufferScrollIndicator(bool s) { if(iShowBufferScrollIndicator!=s) { iShowBufferScrollIndicator=s; emit showBufferScrollIndicatorChanged(); } }

    Q_INVOKABLE QPoint cursorPixelPos();
    Q_INVOKABLE QSize cursorPixelSize();

    bool allowGestures();
    void setAllowGestures(bool allow);

signals:
    void fontSizeChanged();
    void showBufferScrollIndicatorChanged();
    void allowGesturesChanged();

public slots:
    void redraw();
    void updateTermSize();
    void mousePress(float eventX, float eventY);
    void mouseMove(float eventX, float eventY);
    void mouseRelease(float eventX, float eventY);

private slots:
    void handleScrollBack(bool reset);

private:
    Q_DISABLE_COPY(TextRender)

    enum PanGesture { PanNone, PanLeft, PanRight, PanUp, PanDown };

    void paintFromBuffer(QPainter* painter, QList<QList<TermChar> >& buffer, int from, int to, int &y);
    void drawBgFragment(QPainter* painter, int x, int y, int width, TermChar style);
    void drawTextFragment(QPainter* painter, int x, int y, QString text, TermChar style);
    QPoint charsToPixels(QPoint pos);
    void selectionHelper(QPointF scenePos, bool selectionOngoing);

    /**
     * Scroll the back buffer on drag.
     *
     * @param now The current position
     * @param last The last position (or start position)
     * @return The new value for last (modified by any consumed offset)
     **/
    QPointF scrollBackBuffer(QPointF now, QPointF last);
    void doGesture(PanGesture gesture);

    bool newSelection;
    QPointF dragOrigin;

    QFont iFont;
    int iFontWidth;
    int iFontHeight;
    int iFontDescent;
    bool iShowBufferScrollIndicator;
    bool iAllowGestures;

    static Terminal *sTerm;
    static Util *sUtil;

    QList<QColor> iColorTable;
    QImage m_buffer;
};

#endif // TEXTRENDER_H
