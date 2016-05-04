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
#include <QQmlContext>
#include "mainwindow.h"

MainWindow::MainWindow()
{
    rootContext()->setContextProperty("windowHasFocus", false);
}

MainWindow::~MainWindow()
{
}

void MainWindow::focusInEvent(QFocusEvent *event)
{
    rootContext()->setContextProperty("windowHasFocus", true);
    QQuickView::focusInEvent(event);
    emit focusChanged(true);
}

void MainWindow::focusOutEvent(QFocusEvent *event)
{
    rootContext()->setContextProperty("windowHasFocus", false);
    QQuickView::focusOutEvent(event);
    emit focusChanged(false);
}
