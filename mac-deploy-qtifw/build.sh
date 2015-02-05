#!/bin/sh

#  go to the dir containing this script
cd "`dirname "${0}"`"

#  remove previous built installer
rm -rf ./GSView-Installer

#  update our copy of the release build
rm -rf ./packages/com.artifex.gsview/data/gsview.app
cp -r ../build/release/gsview.app ./packages/com.artifex.gsview/data/gsview.app

#  build the installer
./binarycreator -c config/config.xml -p packages GSView-Installer

#  add a missing NIB (this is a qtifw bug)
cp -r ./qt_menu.nib GSView-Installer.app/Contents/Resources/qt_menu.nib
