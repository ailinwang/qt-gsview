#!/bin/sh

#  go to the dir containing this script
this_dir=`dirname "${0}"`
cd "$this_dir"

#  change this to reflect the location of Qt's installer framework
INSTFW=/Users/fredross-perry/Qt/QtIFW-3.0.4

# get latest files
rm -rf ./packages/com.artifex.gsview/data/*
cp -R ../build/release/gsview.app "./packages/com.artifex.gsview/data/GSView 6.0.app"
cp ../UserGuide.pdf ./packages/com.artifex.gsview/data/UserGuide.pdf
cp auto_uninstall.qs ./packages/com.artifex.gsview/data/auto_uninstall.qs

#  build the installer
${INSTFW}/bin/binarycreator -c config/config.xml -p packages GSView-6.0-Installer

#  delete the now-not-needed files
rm -rf ./packages/com.artifex.gsview/data/*
