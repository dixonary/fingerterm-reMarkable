#!/bin/bash
changelog="qtc_packaging/debian_harmattan/changelog"

ver=`cat $changelog | sed -n -e '1p'| cut -d ' ' -f 2 | tr -d "()" | cut -d '-' -f 1`

echo -e \
"#ifndef VERSION_H\n"\
"#define VERSION_H\n"\
"const QString PROGRAM_VERSION=\"$ver\";\n"\
"#endif\n"\
> version.h
