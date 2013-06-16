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

#include <QCoreApplication>

extern "C" {
#include <pty.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
}

#include "terminal.h"
#include "ptyiface.h"

static bool childProcessQuit = false;
static int childProcessPid = 0;

void sighandler(int sig)
{
    if(sig==SIGCHLD) {
        int pid = wait(NULL);

        if(pid > 0 && childProcessPid > 0 &&  pid==childProcessPid) {
            childProcessQuit = true;
            childProcessPid = 0;
            qApp->quit();
        }
    }
}

PtyIFace::PtyIFace(int pid, int masterFd, Terminal *term, QString charset, QObject *parent) :
    QObject(parent),
    iTerm(term),
    iPid(pid),
    iMasterFd(masterFd),
    iFailed(false),
    iReadNotifier(0),
    iTextCodec(0)
{
    childProcessPid = iPid;

    if(!iTerm || childProcessQuit) {
        iFailed = true;
        qFatal("PtyIFace: null Terminal pointer");
    }

    iTerm->setPtyIFace(this);

    resize(iTerm->termSize());
    connect(iTerm,SIGNAL(termSizeChanged(QSize)),this,SLOT(resize(QSize)));

    iReadNotifier = new QSocketNotifier(iMasterFd, QSocketNotifier::Read, this);
    connect(iReadNotifier,SIGNAL(activated(int)),this,SLOT(readActivated()));

    signal(SIGCHLD,&sighandler);
    fcntl(iMasterFd, F_SETFL, O_NONBLOCK); // reads from the descriptor should be non-blocking

    if (!charset.isEmpty())
        iTextCodec = QTextCodec::codecForName(charset.toLatin1());
    if (!iTextCodec)
        iTextCodec = QTextCodec::codecForName("UTF-8");
    if (!iTextCodec)
        qFatal("No valid text codec");
}

PtyIFace::~PtyIFace()
{
    if(!childProcessQuit) {
        // make the process quit
        kill(iPid, SIGHUP);
        kill(iPid, SIGTERM);
        int status=0;
        waitpid(-1,&status,0);
    }
}

void PtyIFace::readActivated()
{
    QByteArray data;
    readTerm(data);
    if(iTerm)
        iTerm->insertInBuffer( iTextCodec->toUnicode(data) );
}

void PtyIFace::resize(QSize newSize)
{
    if(childProcessQuit)
        return;

    winsize winp;
    winp.ws_col = newSize.width();
    winp.ws_row = newSize.height();

    ioctl(iMasterFd, TIOCSWINSZ, &winp);
}

void PtyIFace::writeTerm(const QString &chars)
{
    writeTerm( iTextCodec->fromUnicode(chars) );
}

void PtyIFace::writeTerm(const QByteArray &chars)
{
    if(childProcessQuit)
        return;

    int ret = write(iMasterFd, chars, chars.size());
    if(ret != chars.size())
        qDebug() << "write error!";
}

void PtyIFace::readTerm(QByteArray &chars)
{
    if(childProcessQuit)
        return;

    int ret = 0;
    char ch[64];
    while(ret != -1) {
        ret = read(iMasterFd, &ch, 64);
        if(ret > 0)
            chars.append((char*)&ch, ret);
    }
}
