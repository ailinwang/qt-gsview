#!/bin/bash

cd `dirname "${0}"`

#  edit these lines depending on your setup.
QTDIR=$HOME/Qt5.4.1/5.4/clang_64


# the 2 lines below are just for verification/diagnostics
otool -L ./GSView.app/Contents/MacOS/GSView
codesign --verify --verbose=4 ./GSView.app


