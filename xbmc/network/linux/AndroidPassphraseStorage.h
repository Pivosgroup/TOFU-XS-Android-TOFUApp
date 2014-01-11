#pragma  once
/*
 *      Copyright (C) 2005-2010 Team XBMC
 *      http://www.xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "network/IPassphraseStorage.h"
class CJNIWifiConfiguration;

class CAndroidPassphraseStorage : public IPassphraseStorage
{
public:
  virtual ~CAndroidPassphraseStorage(){};
  virtual bool HasStoredPassphrase(const std::string &uuid, IConnection *connection);
  virtual void InvalidatePassphrase(const std::string &uuid, IConnection *connection = NULL);
  virtual bool GetPassphrase(const std::string &uuid, std::string &passphrase, IConnection *connection = NULL);
  virtual void StorePassphrase(const std::string &uuid, const std::string &passphrase, IConnection *connection = NULL);
};
