#!/bin/sh

#  go to the dir containing this script
this_dir=`dirname "${0}"`
cd "$this_dir"

# refresh release build parts
rm -f ./packages/com.artifex.gsview/data/gsview
rm -rf ./packages/com.artifex.gsview/data/apps
mkdir ./packages/com.artifex.gsview/data/apps
cp ../build/release/gsview ./packages/com.artifex.gsview/data/gsview
cp -r ../build/release/apps ./packages/com.artifex.gsview/data/apps

./binarycreator -c config/config.xml -p packages gsview-installer
