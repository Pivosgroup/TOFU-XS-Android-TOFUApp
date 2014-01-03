@echo off
ECHO ----------------------------------------
echo Creating TOFU Build Folder
IF Exist ..\..\project\Win32BuildSetup\BUILD_WIN32\Xbmc\addons\skin.tofu rmdir ..\..\project\Win32BuildSetup\BUILD_WIN32\Xbmc\addons\skin.tofu /S /Q
md ..\..\project\Win32BuildSetup\BUILD_WIN32\Xbmc\addons\skin.tofu\media\

Echo .svn>exclude.txt
Echo Thumbs.db>>exclude.txt
Echo Desktop.ini>>exclude.txt
Echo dsstdfx.bin>>exclude.txt
Echo build.bat>>exclude.txt
Echo \skin.tofu\media\>>exclude.txt
Echo exclude.txt>>exclude.txt

ECHO ----------------------------------------
ECHO Creating XBT File...
START /B /WAIT ..\..\Tools\TexturePacker\TexturePacker -dupecheck -input media -output ..\..\project\Win32BuildSetup\BUILD_WIN32\Xbmc\addons\skin.tofu\media\Textures.xbt

ECHO ----------------------------------------
ECHO XBT Texture Files Created...
ECHO Building Skin Directory...
xcopy "..\skin.tofu" "..\..\project\Win32BuildSetup\BUILD_WIN32\Xbmc\addons\skin.tofu" /E /Q /I /Y /EXCLUDE:exclude.txt

del exclude.txt
