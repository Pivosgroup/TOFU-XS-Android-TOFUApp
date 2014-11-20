# -*- coding: utf-8 -*- 

import os
import sys
import xbmc
import xbmcaddon,xbmcvfs, math, array,zlib,struct

try:
  # Python 2.6 +
  from hashlib import md5 as md5
  from hashlib import sha256
except ImportError:
  # Python 2.5 and earlier
  from md5 import md5
  from sha256 import sha256

from PIL import Image

__addon__      = xbmcaddon.Addon()
__author__     = __addon__.getAddonInfo('author')
__scriptid__   = __addon__.getAddonInfo('id')
__scriptname__ = __addon__.getAddonInfo('name')
__cwd__        = __addon__.getAddonInfo('path')
__version__    = __addon__.getAddonInfo('version')
__language__   = __addon__.getLocalizedString

__cwd__        = xbmc.translatePath( __addon__.getAddonInfo('path') ).decode("utf-8")
__profile__    = xbmc.translatePath( __addon__.getAddonInfo('profile') ).decode("utf-8")
__resource__   = os.path.join( __cwd__, u'resources', u'lib' )

sys.path.append (__resource__)

BW = os.path.join( __cwd__, "test_BW.jpg")

xbmcvfs.delete(BW)

image_file = Image.open(os.path.join( __cwd__, "test.jpg")) # open colour image
image_file = image_file.convert('1') # convert image to black and white
image_file.save(BW)
