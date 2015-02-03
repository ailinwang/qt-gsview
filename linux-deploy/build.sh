#!/bin/sh

#  go to the dir containing this script
this_dir=`dirname "${0}"`
cd "$this_dir"

./binarycreator -c config/config.xml -p packages gsview-installer
