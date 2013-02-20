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
#include "textrender.h"
#include "terminal.h"
#include "util.h"

TextRender::TextRender(QDeclarativeItem *parent) :
    QDeclarativeItem(parent),
    iTerm(0),
    iUtil(0)
{
    setFlag(QGraphicsItem::ItemHasNoContents, false);

    connect(this,SIGNAL(widthChanged(int)),this,SLOT(updateTermSize()));
    connect(this,SIGNAL(heightChanged(int)),this,SLOT(updateTermSize()));
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

    if(iColorTable.size()!=16)
        qFatal("invalid color table");

    iShowBufferScrollIndicator = false;

    // caching results in considerably faster redrawing during animations
    setCacheMode(QGraphicsItem::DeviceCoordinateCache);
}

TextRender::~TextRender()
{
}

void TextRender::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    if (!iTerm)
        return;

    painter->save();
    painter->setFont(iFont);

    int y=0;
    if (iTerm->backBufferScrollPos() != 0 && iTerm->backBuffer().size()>0) {
        int from = iTerm->backBuffer().size() - iTerm->backBufferScrollPos();
        if(from<0)
            from=0;
        int to = iTerm->backBuffer().size();
        if(to-from > iTerm->termSize().height())
            to = from + iTerm->termSize().height();
        paintFromBuffer(painter, iTerm->backBuffer(), from, to, y);
        if(to-from < iTerm->termSize().height() && iTerm->buffer().size()>0) {
            int to2 = iTerm->termSize().height() - (to-from);
            if(to2 > iTerm->buffer().size())
                to2 = iTerm->buffer().size();
            paintFromBuffer(painter, iTerm->buffer(), 0, to2, y);
        }
    } else {
        int count = qMin(iTerm->termSize().height(), iTerm->buffer().size());
        paintFromBuffer(painter, iTerm->buffer(), 0, count, y);
    }

    // cursor
    if (iTerm->showCursor()) {
        painter->setOpacity(1.0);
        QPoint cursor = cursorPixelPos();
        QSize csize = cursorPixelSize();
        painter->setPen( iColorTable[Terminal::defaultFgColor] );
        painter->setBrush(Qt::transparent);
        painter->drawRect(cursor.x(), cursor.y(), csize.width(), csize.height());
    }

    // selection
    QRect selection = iTerm->selection();
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
            end = charsToPixels(QPoint(iTerm->termSize().width(), selection.top()));
            painter->drawRect(start.x(), start.y(),
                              end.x()-start.x()+fontWidth(), end.y()-start.y()+fontHeight());

            start = charsToPixels(QPoint(1, selection.top()+1));
            end = charsToPixels(QPoint(iTerm->termSize().width(), selection.bottom()-1));
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

    TermChar tmp = iTerm->zeroChar;
    TermChar nextAttrib = iTerm->zeroChar;
    TermChar currAttrib = iTerm->zeroChar;
    int currentX = leftmargin;
    for(int i=from; i<to; i++) {
        y += iFontHeight;

        if(y >= cutAfter)
            painter->setOpacity(0.3);
        else
            painter->setOpacity(1.0);

        int xcount = qMin(buffer.at(i).count(), iTerm->termSize().width());

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
    update(boundingRect());
}

void TextRender::setTerminal(Terminal *term)
{
    if (!iUtil)
        qFatal("textrender: util class not set");

    iTerm = term;

    iFont = QFont(iUtil->settingsValue("ui/fontFamily").toString(),
                  iUtil->settingsValue("ui/fontSize").toInt());
    iFont.setBold(false);
    QFontMetrics fontMetrics(iFont);
    iFontHeight = fontMetrics.height();
    iFontWidth = fontMetrics.maxWidth();
    iFontDescent = fontMetrics.descent();
}

void TextRender::updateTermSize()
{
    if (!iTerm)
        return;

    QSize s((iWidth-4)/iFontWidth, (iHeight-4)/iFontHeight);
    iTerm->setTermSize(s);
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

        iUtil->setSettingsValue("ui/fontSize", psize);

        emit fontSizeChanged();
    }
}

QPoint TextRender::cursorPixelPos()
{
    QPoint ret = charsToPixels(iTerm->cursorPos());
    ret.rx() += 2;
    ret.ry() += iFontHeight/2;
    return ret;
}

QPoint TextRender::charsToPixels(QPoint pos)
{
    // not 100% accurrate with all fonts
    return QPoint(2+(pos.x()-1)*iFontWidth, (pos.y()-1)*iFontHeight+iFontDescent+1);
}

QSize TextRender::cursorPixelSize()
{
    return (QSize(iFontWidth, iFontHeight/2));
}
