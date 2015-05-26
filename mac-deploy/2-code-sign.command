#!/bin/bash

cd `dirname "${0}"`

#  edit these lines depending on your setup.
QTDIR=$HOME/Qt5.4.1/5.4/clang_64

# correct .app bundle structure
python ./rebundle.py $QTDIR ./GSView.app

#  sign .app bundle (including frameworks and plugins)
#  why do we have to do this twice?
codesign --deep --force --verify --verbose --sign "Developer ID Application: Artifex Software Inc. (7M9658FA5G)" ./GSView.app
codesign --deep --force --verify --verbose --sign "Developer ID Application: Artifex Software Inc. (7M9658FA5G)" ./GSView.app
