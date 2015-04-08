#!/bin/bash
#
#  This script builds a DMG file for distributing gsview.
#  based on:
#  http://asmaloney.com/2013/07/howto/packaging-a-mac-os-x-application-using-a-dmg/

#  go to the directory containing this script
cd `dirname "${0}"`

#  project is one level up
PROJECT="`dirname "${0}"`/.."

#  set up the particulars
APP_NAME="GSView"
VERSION="6.0"
DMG_BACKGROUND_IMG="dmgbackground.png"
VOL_NAME="${APP_NAME} ${VERSION}"   # volume name will be "SuperCoolApp 1.0.0"
DMG_TMP="${VOL_NAME}-temp.dmg.sparseimage"
DMG_FINAL="${VOL_NAME}.dmg"         # final DMG name will be "SuperCoolApp 1.0.0.dmg"
STAGING_DIR="./Install"             # we copy all our stuff into this dir


#  Check the background image DPI and convert it if it isn't 72x72
_BACKGROUND_IMAGE_DPI_H=`sips -g dpiHeight ${DMG_BACKGROUND_IMG} | grep -Eo '[0-9]+\.[0-9]+'`
_BACKGROUND_IMAGE_DPI_W=`sips -g dpiWidth ${DMG_BACKGROUND_IMG} | grep -Eo '[0-9]+\.[0-9]+'`
if [ $(echo " $_BACKGROUND_IMAGE_DPI_H != 72.0 " | bc) -eq 1 -o $(echo " $_BACKGROUND_IMAGE_DPI_W != 72.0 " | bc) -eq 1 ]; then
   echo "WARNING: The background image's DPI is not 72.  This will result in distorted backgrounds on Mac OS X 10.7+."
   echo "         I will convert it to 72 DPI for you."
   
   _DMG_BACKGROUND_TMP="${DMG_BACKGROUND_IMG%.*}"_dpifix."${DMG_BACKGROUND_IMG##*.}"
 
   sips -s dpiWidth 72 -s dpiHeight 72 ${DMG_BACKGROUND_IMG} --out ${_DMG_BACKGROUND_TMP}
   
   DMG_BACKGROUND_IMG="${_DMG_BACKGROUND_TMP}"
fi


#  clear out any previous data
rm -rf "${STAGING_DIR}" "${DMG_TMP}" "${DMG_FINAL}"


#  make staging dir and copy in necessary files.
mkdir -p "${STAGING_DIR}"
cp -rfp ${PROJECT}/build/release/gsview.app "${STAGING_DIR}"
cp -rfp ./license.txt "${STAGING_DIR}"

 
#  create and mount the temporary DMG file
hdiutil create -srcfolder "${STAGING_DIR}" -volname "${VOL_NAME}" -format UDSP "${DMG_TMP}"
DEVICE=$(hdiutil attach -readwrite -noverify "${DMG_TMP}" | egrep '^/dev/' | sed 1q | awk '{print $1}')
sleep 2


#  add a link to the Applications dir
echo "Add link to /Applications"
pushd /Volumes/"${VOL_NAME}"
ln -s /Applications
popd
sleep 2


#  add the background image
mkdir /Volumes/"${VOL_NAME}"/.background
cp "${DMG_BACKGROUND_IMG}" /Volumes/"${VOL_NAME}"/.background/
sleep 2


#  tell Finder to resize the window, set the background,
#  change the icon size, place the icons in the right position, etc.
echo '
   tell application "Finder"
     tell disk "'${VOL_NAME}'"
           open
           set current view of container window to icon view
           set toolbar visible of container window to false
           set statusbar visible of container window to false
           set the bounds of container window to {400, 100, 920, 490}
	   delay 2
           set viewOptions to the icon view options of container window
           set arrangement of viewOptions to not arranged
           set icon size of viewOptions to 72
	   set background picture of viewOptions to file ".background:'${DMG_BACKGROUND_IMG}'"
           set position of item "gsview.app" of container window to {160, 190}
           set position of item "Applications" of container window to {360, 190}
           set position of item "license.txt" of container window to {260, 300}
           close
           delay 2
           open
           update without registering applications
           delay 2
     end tell
   end tell
' | osascript
 
sync
sleep 2


#  bless the folder so it opens automatically
#  and then unmount
bless /Volumes/${VOL_NAME} --openfolder /Volumes/${VOL_NAME}
hdiutil detach "${DEVICE}"


#  now make the final image using compression
echo "Creating compressed image"
hdiutil convert "${DMG_TMP}" -format UDZO -imagekey zlib-level=9 -o "${DMG_FINAL}"
 

# clean up
rm -rf "${DMG_TMP}"
rm -rf "${STAGING_DIR}"
 

echo 'Done.'
exit


#  done.
