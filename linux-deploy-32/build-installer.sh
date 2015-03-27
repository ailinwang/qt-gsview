#!/bin/sh

#  go to the dir containing this script
this_dir=`dirname "${0}"`
cd "$this_dir"

# get latest built apps
rm -f ./packages/com.artifex.gsview/data/gsview
rm -rf ./packages/com.artifex.gsview/data/apps
cp ../build/release/gsview ./packages/com.artifex.gsview/data/gsview
cp -r ../build/release/apps ./packages/com.artifex.gsview/data/apps

#  build the installer
./binarycreator -c config/config.xml -p packages GSView-6.0-Installer-32

#  delete the now-not-needed apps
rm -f ./packages/com.artifex.gsview/data/gsview
rm -rf ./packages/com.artifex.gsview/data/apps

