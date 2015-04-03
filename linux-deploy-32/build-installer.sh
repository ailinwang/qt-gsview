#!/bin/sh

#  go to the dir containing this script
this_dir=`dirname "${0}"`
cd "$this_dir"

# get latest built apps
rm -f ./packages/com.artifex.gsview/data/gsview.exe
rm -rf ./packages/com.artifex.gsview/data/apps
cp ../build/release/gsview ./packages/com.artifex.gsview/data/gsview.exe
cp -r ../build/release/apps ./packages/com.artifex.gsview/data/apps

#  patch the exe and set permissions
patchelf --set-interpreter ./libs/ld-linux.so.2 ./packages/com.artifex.gsview/data/gsview.exe 
chmod 755 ./packages/com.artifex.gsview/data/gsview.exe
chmod 755 ./packages/com.artifex.gsview/data/gsview

#  build the installer
./binarycreator -c config/config.xml -p packages GSView-6.0-Installer-32
chmod 755 GSView-6.0-Installer-32

#  delete the now-not-needed apps
rm -f ./packages/com.artifex.gsview/data/gsview.exe
rm -rf ./packages/com.artifex.gsview/data/apps

