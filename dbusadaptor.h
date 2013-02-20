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

#ifndef DBUSADAPTOR_H
#define DBUSADAPTOR_H

#include "qplatformdefs.h"
class MainWindow;

#ifdef MEEGO_EDITION_HARMATTAN

// handles dbus registration & multiple instances on harmattan

#include <MApplicationService>

class DbusAdaptor : public MApplicationService
{
    Q_OBJECT

public:
    explicit DbusAdaptor(QObject *parent = 0);
    virtual ~DbusAdaptor();

public slots:
    virtual void launch();
    virtual void launch(const QStringList &parameters);
    virtual void handleServiceRegistrationFailure();

    void setAppWindow(MainWindow* win) { mainWin = win; }

private:
    Q_DISABLE_COPY(DbusAdaptor)

    MainWindow* mainWin;
};

#endif // MEEGO_EDITION_HARMATTAN
#endif // DBUSADAPTOR_H
