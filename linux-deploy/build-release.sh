#!/bin/bash

#
#  This script builds a Linux Release version of gsview.
#  It relies on external factors:
#
#  1.  The QT project has been set up to put a release buid in "build/release"
#  2.  The Qt project is currently set to make a release build

#  edit this line depending on your setup.
QTBIN=$HOME/Qt5.4.1/5.4/gcc_64/bin

#  project is one level up
PROJECT="`dirname "${0}"`/.."

#  remove any old release build.
cd $PROJECT/build
rm -rf ./release

#  create a new empty release folder.
mkdir ./release
cd ./release

#  make the makefile.
$QTBIN/qmake ../../gsview.pro -r -spec linux-g++ CONFIG+=x86_64

#  make the app
make clean
make

#  done.
