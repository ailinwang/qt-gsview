#!/bin/sh

#  go to the dir containing this script
this_dir=`dirname "${0}"`
cd "$this_dir"

# get latest built apps
rm -f ./packages/com.artifex.gsview/data/gsview.exe
rm -rf ./packages/com.artifex.gsview/data/apps
rm -rf ./packages/com.artifex.gsview/data/UserGuide.pdf
cp ../build/release/gsview ./packages/com.artifex.gsview/data/gsview.exe
cp -r ../build/release/apps ./packages/com.artifex.gsview/data/apps
cp ../build/release/UserGuide.pdf ./packages/com.artifex.gsview/data/UserGuide.pdf

#  patch the exe and set permissions
patchelf --set-interpreter ./libs/ld-linux-x86-64.so.2 ./packages/com.artifex.gsview/data/gsview.exe 
chmod 755 ./packages/com.artifex.gsview/data/gsview.exe
chmod 755 ./packages/com.artifex.gsview/data/gsview

#  build the installer
./binarycreator -c config/config.xml -p packages GSView-6.0-Installer
chmod 755 GSView-6.0-Installer

#  delete the now-not-needed apps
rm -f ./packages/com.artifex.gsview/data/gsview.exe
rm -rf ./packages/com.artifex.gsview/data/apps

