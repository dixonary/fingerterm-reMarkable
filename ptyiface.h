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

#ifndef PTYIFACE_H
#define PTYIFACE_H

#include <QObject>
#include <QSocketNotifier>
#include <QByteArray>
#include <QSize>
#include <QTextCodec>

class Terminal;

class PtyIFace : public QObject
{
    Q_OBJECT
public:
    explicit PtyIFace(int pid, int masterFd, Terminal *term, QString charset, QObject *parent = 0);
    virtual ~PtyIFace();

    void writeTerm(const QString &chars);
    bool failed() { return iFailed; }

public slots:
    void resize(QSize newSize);

private slots:
    void readActivated();

private:
    Q_DISABLE_COPY(PtyIFace)

    void writeTerm(const QByteArray &chars);
    void readTerm(QByteArray &chars);

    Terminal *iTerm;
    int iPid;
    int iMasterFd;
    bool iFailed;

    QSocketNotifier *iReadNotifier;

    QTextCodec *iTextCodec;
};

#endif // PTYIFACE_H
