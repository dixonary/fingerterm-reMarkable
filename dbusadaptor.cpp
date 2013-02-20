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

#include <QApplication>
#include <QWidget>
#include "dbusadaptor.h"
#include "mainwindow.h"

#ifdef MEEGO_EDITION_HARMATTAN

DbusAdaptor::DbusAdaptor(QObject *parent):
    MApplicationService("org.hqh.fingerterm", parent),
    mainWin(0)
{
}

DbusAdaptor::~DbusAdaptor()
{
}

void DbusAdaptor::launch()
{
    MApplicationService::launch();

    if (mainWin) {
        mainWin->raise();
    }
}

void DbusAdaptor::launch(const QStringList &parameters)
{
    if (parameters.contains("new"))
        launchAnotherWithQProcess();
    else
        launch();
}

void DbusAdaptor::handleServiceRegistrationFailure()
{
    // for some reason the subsequent instances get the default "com.nokia..." prefix
    incrementAndRegister();
}

#endif // MEEGO_EDITION_HARMATTAN
