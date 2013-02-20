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

#include <QtCore>
#include <QDebug>

#include "keyloader.h"
#include "util.h"

KeyLoader::KeyLoader(QObject *parent) :
    QObject(parent),
    iVkbRows(0),
    iVkbColumns(0),
    iUtil(0)
{
}

KeyLoader::~KeyLoader()
{
}

bool KeyLoader::loadLayout(QString layout)
{
    bool ret = false;
    if(layout.isEmpty() || !iUtil)
        return false;

    if (layout.at(0)==':') {  // load from resources
        QResource res(layout);
        QByteArray resArr( reinterpret_cast<const char*>(res.data()) );
        QBuffer resBuf( &resArr );
        ret = loadLayoutInternal(resBuf);
    }
    else { // load from file
        QFile f(iUtil->configPath() + "/" + layout + ".layout");
        ret = loadLayoutInternal(f);
    }

    return ret;
}

bool KeyLoader::loadLayoutInternal(QIODevice &from)
{
    iKeyData.clear();
    bool ret = true;

    iVkbRows = 0;
    iVkbColumns = 0;
    bool lastLineHadKey = false;

    if( !from.open(QIODevice::ReadOnly | QIODevice::Text) )
        return false;

    QList<KeyData> keyRow;
    while(!from.atEnd()) {
        QString line = QString::fromUtf8(from.readLine()).simplified();
        if(line.length()>=2 && line.at(0)!=';' && line.at(0)=='[' && line.at(line.length()-1)==']')
        {
            KeyData key;
            key.label = "";
            key.code = 0;
            key.label_alt = "";
            key.code_alt = 0;
            key.isModifier = false;

            line.replace("\\\\", "\\x5C");
            line.replace("\" \"", "\\x20");
            line.replace(" ", "");
            line.replace("\"[\"", "\\x5B");
            line.replace("\"]\"", "\\x5D");
            line.replace("\",\"", "\\x2C");
            line.replace("\\\"", "\\x22");
            line.replace("\"", "");
            key.width = line.count('[');
            line.replace("[", "");
            line.replace("]", "");

            line.replace("\\x20", " ");
            line.replace("\\x22", "\"");
            line.replace("\\x5B", "[");
            line.replace("\\x5D", "]");
            line.replace("\\x5C", "\\");

            QStringList parts = line.split(",", QString::KeepEmptyParts);
            if(parts.count()>=2) {
                bool ok = true;
                key.label = parts.at(0);
                key.label.replace("\\x2C",",");
                parts[1].replace("0x","");
                key.code = parts.at(1).toInt(&ok,16);
                if(!ok) {
                    ret = false;
                    break;
                }
                if(key.code==Qt::AltModifier || key.code==Qt::ControlModifier || key.code==Qt::ShiftModifier)
                    key.isModifier = true;
                if(parts.count()>=4 && !key.isModifier) {
                    key.label_alt = parts.at(2);
                    key.label_alt.replace("\\x2C",",");
                    parts[3].replace("0x","");
                    key.code_alt = parts.at(3).toInt(&ok,16);
                    if(!ok) {
                        ret = false;
                        break;
                    }
                }
            }
            lastLineHadKey = true;
            cleanUpKey(key);
            keyRow.append(key);
        }
        else if(line.length()==0 && lastLineHadKey) {
            if(keyRow.count() > iVkbColumns) {
                iVkbColumns = keyRow.count();
            }
            iKeyData.append(keyRow);
            keyRow.clear();
            lastLineHadKey = false;
        }
        else {
            lastLineHadKey = false;
        }
    }
    if(keyRow.count() > 0)
        iKeyData.append(keyRow);

    iVkbRows = iKeyData.count();
    foreach(QList<KeyData> r, iKeyData) {
        if(r.count() > iVkbColumns)
            iVkbColumns = r.count();
    }

    from.close();

    if (iVkbColumns <= 0 || iVkbRows <= 0)
        ret = false;

    if (!ret)
        iKeyData.clear();

    return ret;
}

QVariantList KeyLoader::keyAt(int row, int col)
{
    QVariantList ret;
    ret.append(""); //label
    ret.append(0);  //code
    ret.append(""); //label_alt
    ret.append(0);  //code_alt
    ret.append(0);  //width
    ret.append(false);  //isModifier

    if(iKeyData.count() <= row)
        return ret;
    if(iKeyData.at(row).count() <= col)
        return ret;

    ret[0] = iKeyData.at(row).at(col).label;
    ret[1] = iKeyData.at(row).at(col).code;
    ret[2] = iKeyData.at(row).at(col).label_alt;
    ret[3] = iKeyData.at(row).at(col).code_alt;
    ret[4] = iKeyData.at(row).at(col).width;
    ret[5] = iKeyData.at(row).at(col).isModifier;

    return ret;
}

const QStringList KeyLoader::availableLayouts()
{
    if (!iUtil)
        return QStringList();

    QDir confDir(iUtil->configPath());
    QStringList filter("*.layout");

    QStringList results = confDir.entryList(filter, QDir::Files|QDir::Readable, QDir::Name);

    QStringList ret;
    foreach(QString s, results) {
        ret << s.left(s.lastIndexOf('.'));
    }

    return ret;
}

void KeyLoader::cleanUpKey(KeyData &key)
{
    // make sure that a key does not try to use some (currently) unsupported feature...

    // if the label is an image or a modifier, we do not support an alternative label
    if ((key.label.startsWith(':') && key.label.length()>1) || key.isModifier) {
        key.label_alt = "";
        key.code_alt = 0;
    }

    // if the alternative label is an image (and the default one was not), use it as the (only) default
    if (key.label_alt.startsWith(':') && key.label_alt.length()>1) {
        key.label = key.label_alt;
        key.code = key.code_alt;
        key.label_alt = "";
        key.code_alt = 0;
    }

    // alphabet letters can't have an alternative, they just switch between lower and upper case
    if (key.label.length()==1 && key.label.at(0).isLetter()) {
        key.label_alt = "";
        key.code_alt = 0;
    }

    // ... also, can't have alphabet letters as an alternative label
    if (key.label_alt.length()==1 && key.label_alt.at(0).isLetter()) {
        key.label_alt = "";
        key.code_alt = 0;
    }
}
