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
#include "mainwindow.h"

#ifdef MEEGO_EDITION_HARMATTAN
#include <MApplication>
#include <MNotification>
#include <QX11Info>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#endif //MEEGO_EDITION_HARMATTAN

MainWindow::MainWindow()
{
}

MainWindow::~MainWindow()
{
}

void MainWindow::focusInEvent(QFocusEvent *event)
{
    QQuickView::focusInEvent(event);
    emit focusChanged(true);
}

void MainWindow::focusOutEvent(QFocusEvent *event)
{
    QQuickView::focusOutEvent(event);
    emit focusChanged(false);
}

void MainWindow::minimize()
{
    setWindowState(Qt::WindowMinimized);
}

void MainWindow::disableSwipe()
{
#ifdef MEEGO_EDITION_HARMATTAN
    resize(MApplication::desktop()->screenGeometry().width(),
           MApplication::desktop()->screenGeometry().height());
    showFullScreen();

    unsigned int customRegion[] =
    {
        rect().x(),
        rect().y(),
        rect().width(),
        rect().height()
    };

    Display *dpy = QX11Info::display();
    Atom customRegionAtom = XInternAtom(dpy, "_MEEGOTOUCH_CUSTOM_REGION", False);

    XChangeProperty(dpy, effectiveWinId(), customRegionAtom,
                    XA_CARDINAL, 32, PropModeReplace,
                    reinterpret_cast<unsigned char*>(&customRegion[0]), 4);

#endif //MEEGO_EDITION_HARMATTAN
}

void MainWindow::enableSwipe()
{
#ifdef MEEGO_EDITION_HARMATTAN
    Display *dpy = QX11Info::display();
    Atom customRegionAtom = XInternAtom(dpy, "_MEEGOTOUCH_CUSTOM_REGION", False);

    XDeleteProperty(dpy, effectiveWinId(), customRegionAtom);
#endif //MEEGO_EDITION_HARMATTAN
}
