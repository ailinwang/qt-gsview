#!/bin/bash

#
#  This script builds a Mac Release version of gsview.
#  It relies on external factors:
#
#  1.  The QT project has been set up to put a release buid in "build/release"
#  2.  The Qt project is currently set to make a release build
#  3.  qmake has been run

#  edit this line depending on your setup.
QTBIN=$HOME/Qt5.4.1/5.4/clang_64/bin

#  project is one level up
HOME="`dirname "${0}"`"
PROJECT="`dirname "${0}"`/.."

#  remove any old release build.
cd $PROJECT/build
rm -rf ./release

#  create a new empty release folder.
mkdir ./release
cd ./release

#  make the makefile.
$QTBIN/qmake $PROJECT/gsview.pro -r -spec macx-clang CONFIG+=x86_64

#  make the app
make clean
make

#  run Qt's deployment tool which will, among other things,
#  puts Qt's frameworks inside the app.
$QTBIN/macdeployqt ./gsview.app

#  put a copy in the folder that contains this script
cd $HOME
rm -rf ./GSView.app
cp -R ../build/release/GSView.app ./GSView.app


#  done.
