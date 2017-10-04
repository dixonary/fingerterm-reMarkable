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

#include <QtGui>
#include <epframebuffer.h>
#include "textrender.h"
#include "terminal.h"
#include "util.h"

Terminal* TextRender::sTerm = 0;
Util* TextRender::sUtil = 0;

TextRender::TextRender(QQuickItem *parent) :
    QQuickPaintedItem(parent),
    newSelection(true),
    iAllowGestures(true)
{
    setFlag(ItemHasContents);

    connect(this,SIGNAL(widthChanged()),this,SLOT(updateTermSize()));
    connect(this,SIGNAL(heightChanged()),this,SLOT(updateTermSize()));
    connect(this,SIGNAL(fontSizeChanged()),this,SLOT(updateTermSize()));

    //normal
    iColorTable.append(QColor(0, 0, 0));
    iColorTable.append(QColor(210, 0, 0));
    iColorTable.append(QColor(0, 210, 0));
    iColorTable.append(QColor(210, 210, 0));
    iColorTable.append(QColor(0, 0, 240));
    iColorTable.append(QColor(210, 0, 210));
    iColorTable.append(QColor(0, 210, 210));
    iColorTable.append(QColor(235, 235, 235));

    //bright
    iColorTable.append(QColor(127, 127, 127));
    iColorTable.append(QColor(255, 0, 0));
    iColorTable.append(QColor(0, 255, 0));
    iColorTable.append(QColor(255, 255, 0));
    iColorTable.append(QColor(92, 92, 255));
    iColorTable.append(QColor(255, 0, 255));
    iColorTable.append(QColor(0, 255, 255));
    iColorTable.append(QColor(255, 255, 255));

    //colour cube
    for (int r = 0x00; r < 0x100; r += 0x33)
        for (int g = 0x00; g < 0x100; g += 0x33)
            for (int b = 0x00; b < 0x100; b += 0x33)
                iColorTable.append(QColor(r, g, b));

    //greyscale ramp
    int ramp[] = {
          0,  11,  22,  33,  44,  55,  66,  77,  88,  99, 110, 121,
        133, 144, 155, 166, 177, 188, 199, 210, 221, 232, 243, 255
    };
    for (int i = 0; i < 24; i++)
        iColorTable.append(QColor(ramp[i], ramp[i], ramp[i]));

    if(iColorTable.size() != 256)
        qFatal("invalid color table");

    iShowBufferScrollIndicator = false;

    iFont = QFont(sUtil->fontFamily(), sUtil->fontSize());
    iFont.setBold(false);
    QFontMetrics fontMetrics(iFont);
    iFontHeight = fontMetrics.height();
    iFontWidth = fontMetrics.maxWidth();
    iFontDescent = fontMetrics.descent();

    Q_ASSERT(sTerm);
    connect(sTerm, SIGNAL(displayBufferChanged()), this, SLOT(redraw()));
    connect(sTerm, SIGNAL(cursorPosChanged(QPoint)), this, SLOT(redraw()));
    connect(sTerm, SIGNAL(termSizeChanged(int,int)), this, SLOT(redraw()));
    connect(sTerm, SIGNAL(selectionChanged()), this, SLOT(redraw()));
    connect(sTerm, SIGNAL(scrollBackBufferAdjusted(bool)), this, SLOT(handleScrollBack(bool)));
    updateTermSize();
}

TextRender::~TextRender()
{
}

void TextRender::paint(QPainter* painter)
{
    painter->save();
    painter->setFont(iFont);

    int y=0;
    if (sTerm->backBufferScrollPos() != 0 && sTerm->backBuffer().size()>0) {
        int from = sTerm->backBuffer().size() - sTerm->backBufferScrollPos();
        if(from<0)
            from=0;
        int to = sTerm->backBuffer().size();
        if(to-from > sTerm->rows())
            to = from + sTerm->rows();
        paintFromBuffer(painter, sTerm->backBuffer(), from, to, y);
        if(to-from < sTerm->rows() && sTerm->buffer().size()>0) {
            int to2 = sTerm->rows() - (to-from);
            if(to2 > sTerm->buffer().size())
                to2 = sTerm->buffer().size();
            paintFromBuffer(painter, sTerm->buffer(), 0, to2, y);
        }
    } else {
        int count = qMin(sTerm->rows(), sTerm->buffer().size());
        paintFromBuffer(painter, sTerm->buffer(), 0, count, y);
    }

    // cursor
    if (sTerm->showCursor()) {
        painter->setOpacity(0.5);
        QPoint cursor = cursorPixelPos();
        QSize csize = cursorPixelSize();
        painter->setPen(Qt::transparent);
        painter->setBrush(iColorTable[Terminal::defaultFgColor]);
        painter->drawRect(cursor.x(), cursor.y(), csize.width(), csize.height());
    }

    // selection
    QRect selection = sTerm->selection();
    if (!selection.isNull()) {
        painter->setOpacity(0.5);
        painter->setPen(Qt::transparent);
        painter->setBrush(Qt::blue);
        QPoint start, end;

        if (selection.top() == selection.bottom()) {
            start = charsToPixels(selection.topLeft());
            end = charsToPixels(selection.bottomRight());
            painter->drawRect(start.x(), start.y(),
                              end.x()-start.x()+fontWidth(), end.y()-start.y()+fontHeight());
        } else {
            start = charsToPixels(selection.topLeft());
            end = charsToPixels(QPoint(sTerm->columns(), selection.top()));
            painter->drawRect(start.x(), start.y(),
                              end.x()-start.x()+fontWidth(), end.y()-start.y()+fontHeight());

            start = charsToPixels(QPoint(1, selection.top()+1));
            end = charsToPixels(QPoint(sTerm->columns(), selection.bottom()-1));
            painter->drawRect(start.x(), start.y(),
                              end.x()-start.x()+fontWidth(), end.y()-start.y()+fontHeight());

            start = charsToPixels(QPoint(1, selection.bottom()));
            end = charsToPixels(selection.bottomRight());
            painter->drawRect(start.x(), start.y(),
                              end.x()-start.x()+fontWidth(), end.y()-start.y()+fontHeight());
        }
    }

    painter->restore();
}

void TextRender::paintFromBuffer(QPainter* painter, QList<QList<TermChar> >& buffer, int from, int to, int &y)
{
    const int leftmargin = 2;
    int cutAfter = property("cutAfter").toInt() + iFontDescent;

    TermChar tmp = sTerm->zeroChar;
    TermChar nextAttrib = sTerm->zeroChar;
    TermChar currAttrib = sTerm->zeroChar;
    int currentX = leftmargin;
    for(int i=from; i<to; i++) {
        y += iFontHeight;

        if(y >= cutAfter)
            painter->setOpacity(0.3);
        else
            painter->setOpacity(1.0);

        int xcount = qMin(buffer.at(i).count(), sTerm->columns());

        // background for the current line
        currentX = leftmargin;
        int fragWidth = 0;
        for(int j=0; j<xcount; j++) {
            tmp = buffer[i][j];
            fragWidth += iFontWidth;
            if (j==0) {
                currAttrib = tmp;
                nextAttrib = tmp;
            } else if (j<xcount-1) {
                nextAttrib = buffer[i][j+1];
            }

            if (currAttrib.attrib != nextAttrib.attrib ||
                currAttrib.bgColor != nextAttrib.bgColor ||
                currAttrib.fgColor != nextAttrib.fgColor ||
                j==xcount-1)
            {
                drawBgFragment(painter, currentX, y-iFontHeight+iFontDescent, fragWidth, currAttrib);
                currentX += fragWidth;
                fragWidth = 0;
                currAttrib.attrib = nextAttrib.attrib;
                currAttrib.bgColor = nextAttrib.bgColor;
                currAttrib.fgColor = nextAttrib.fgColor;
            }
        }

        // text for the current line
        QString line;
        currentX = leftmargin;
        for (int j=0; j<xcount; j++) {
            tmp = buffer[i][j];
            line += tmp.c;
            if (j==0) {
                currAttrib = tmp;
                nextAttrib = tmp;
            } else if(j<xcount-1) {
                nextAttrib = buffer[i][j+1];
            }

            if (currAttrib.attrib != nextAttrib.attrib ||
                currAttrib.bgColor != nextAttrib.bgColor ||
                currAttrib.fgColor != nextAttrib.fgColor ||
                j==xcount-1)
            {
                drawTextFragment(painter, currentX, y, line, currAttrib);
                currentX += iFontWidth*line.length();
                line.clear();
                currAttrib.attrib = nextAttrib.attrib;
                currAttrib.bgColor = nextAttrib.bgColor;
                currAttrib.fgColor = nextAttrib.fgColor;
            }
        }
    }
}

void TextRender::drawBgFragment(QPainter* painter, int x, int y, int width, TermChar style)
{
    if (style.attrib & attribNegative) {
        int c = style.fgColor;
        style.fgColor = style.bgColor;
        style.bgColor = c;
    }

    if (style.bgColor == Terminal::defaultBgColor)
        return;

    painter->setPen(Qt::transparent);
    painter->setBrush( iColorTable[style.bgColor] );
    painter->drawRect(x, y, width, iFontHeight);
}

void TextRender::drawTextFragment(QPainter* painter, int x, int y, QString text, TermChar style)
{
    if (style.attrib & attribNegative) {
        int c = style.fgColor;
        style.fgColor = style.bgColor;
        style.bgColor = c;
    }
    if (style.attrib & attribBold) {
        iFont.setBold(true);
        painter->setFont(iFont);
        if(style.fgColor < 8)
            style.fgColor += 8;
    } else if(iFont.bold()) {
        iFont.setBold(false);
        painter->setFont(iFont);
    }

    painter->setPen( iColorTable[style.fgColor] );
    painter->setBrush(Qt::transparent);
    painter->drawText(x, y, text);
}

void TextRender::redraw()
{
//    EPFrameBuffer::framebuffer()->fill(Qt::black);
    QPainter painter(EPFrameBuffer::framebuffer());
    const QRect rect(0, 0, width(), height());
    QImage &buffer = m_buffer;
    if (buffer.size() != rect.size()) {
        buffer = QImage();
    }
    painter.fillRect(rect, Qt::black);
    paint(&painter);
    EPFrameBuffer::sendUpdate(rect, EPFrameBuffer::Grayscale, EPFrameBuffer::PartialUpdate, false);
//    update();
}

void TextRender::setUtil(Util *util)
{
    sUtil = util;
}

void TextRender::setTerminal(Terminal *terminal)
{
    sTerm = terminal;
}

void TextRender::updateTermSize()
{
    QSize size((width() - 4) / iFontWidth, (height() - 4) / iFontHeight);
    sTerm->setTermSize(size);
}

void TextRender::mousePress(float eventX, float eventY)
{
    if (!allowGestures())
        return;

    dragOrigin = QPointF(eventX, eventY);
    newSelection = true;
}

void TextRender::mouseMove(float eventX, float eventY)
{
    QPointF eventPos(eventX, eventY);

    if (!allowGestures())
        return;

    if(sUtil->dragMode() == Util::DragScroll) {
        dragOrigin = scrollBackBuffer(eventPos, dragOrigin);
    }
    else if(sUtil->dragMode() == Util::DragSelect) {
        selectionHelper(eventPos, true);
    }
}

void TextRender::mouseRelease(float eventX, float eventY)
{
    QPointF eventPos(eventX, eventY);
    const int reqDragLength = 140;

    if (!allowGestures())
        return;

    if(sUtil->dragMode() == Util::DragGestures) {
        int xdist = qAbs(eventPos.x() - dragOrigin.x());
        int ydist = qAbs(eventPos.y() - dragOrigin.y());
        if(eventPos.x() < dragOrigin.x()-reqDragLength && xdist > ydist*2)
            doGesture(PanLeft);
        else if(eventPos.x() > dragOrigin.x()+reqDragLength && xdist > ydist*2)
            doGesture(PanRight);
        else if(eventPos.y() > dragOrigin.y()+reqDragLength && ydist > xdist*2)
            doGesture(PanDown);
        else if(eventPos.y() < dragOrigin.y()-reqDragLength && ydist > xdist*2)
            doGesture(PanUp);
    }
    else if(sUtil->dragMode() == Util::DragScroll) {
        scrollBackBuffer(eventPos, dragOrigin);
    }
    else if(sUtil->dragMode() == Util::DragSelect) {
        selectionHelper(eventPos, false);
    }
}

void TextRender::selectionHelper(QPointF scenePos, bool selectionOngoing)
{
    int yCorr = fontDescent();

    QPoint start(qRound((dragOrigin.x()+2) / fontWidth()),
                 qRound((dragOrigin.y()+yCorr) / fontHeight()));
    QPoint end(qRound((scenePos.x()+2) / fontWidth()),
               qRound((scenePos.y()+yCorr) / fontHeight()));

    if (start != end) {
        sTerm->setSelection(start, end, selectionOngoing);
        newSelection = false;
    }
}

void TextRender::handleScrollBack(bool reset)
{
    if (reset) {
        setShowBufferScrollIndicator(false);
    } else {
        setShowBufferScrollIndicator(sTerm->backBufferScrollPos() != 0);
    }
    redraw();
}

void TextRender::setFontPointSize(int psize)
{
    if (iFont.pointSize() != psize)
    {
        iFont.setPointSize(psize);
        QFontMetrics fontMetrics(iFont);
        iFontHeight = fontMetrics.height();
        iFontWidth = fontMetrics.maxWidth();
        iFontDescent = fontMetrics.descent();
        emit fontSizeChanged();
    }
}

QPoint TextRender::cursorPixelPos()
{
    return charsToPixels(sTerm->cursorPos());
}

QPoint TextRender::charsToPixels(QPoint pos)
{
    // not 100% accurrate with all fonts
    return QPoint(2+(pos.x()-1)*iFontWidth, (pos.y()-1)*iFontHeight+iFontDescent+1);
}

QSize TextRender::cursorPixelSize()
{
    return QSize(iFontWidth, iFontHeight);
}

bool TextRender::allowGestures()
{
    return iAllowGestures;
}

void TextRender::setAllowGestures(bool allow)
{
    if (iAllowGestures != allow) {
        iAllowGestures = allow;
        emit allowGesturesChanged();
    }
}

QPointF TextRender::scrollBackBuffer(QPointF now, QPointF last)
{
    int xdist = qAbs(now.x() - last.x());
    int ydist = qAbs(now.y() - last.y());
    int fontSize = fontPointSize();

    int lines = ydist / fontSize;

    if(lines > 0 && now.y() < last.y() && xdist < ydist*2) {
        sTerm->scrollBackBufferFwd(lines);
        last = QPointF(now.x(), last.y() - lines * fontSize);
    } else if(lines > 0 && now.y() > last.y() && xdist < ydist*2) {
        sTerm->scrollBackBufferBack(lines);
        last = QPointF(now.x(), last.y() + lines * fontSize);
    }

    return last;
}

void TextRender::doGesture(PanGesture gesture)
{
    if( gesture==PanLeft ) {
        sUtil->notifyText(sUtil->settingsValue("gestures/panLeftTitle", "Alt-Right").toString());
        sTerm->putString(sUtil->settingsValue("gestures/panLeftCommand", "\\e\\e[C").toString(), true);
    }
    else if( gesture==PanRight ) {
        sUtil->notifyText(sUtil->settingsValue("gestures/panRightTitle", "Alt-Left").toString());
        sTerm->putString(sUtil->settingsValue("gestures/panRightCommand", "\\e\\e[D").toString(), true);
    }
    else if( gesture==PanDown ) {
        sUtil->notifyText(sUtil->settingsValue("gestures/panDownTitle", "Page Up").toString());
        sTerm->putString(sUtil->settingsValue("gestures/panDownCommand", "\\e[5~").toString(), true);
    }
    else if( gesture==PanUp ) {
        sUtil->notifyText(sUtil->settingsValue("gestures/panUpTitle", "Page Down").toString());
        sTerm->putString(sUtil->settingsValue("gestures/panUpCommand", "\\e[6~").toString(), true);
    }
}
