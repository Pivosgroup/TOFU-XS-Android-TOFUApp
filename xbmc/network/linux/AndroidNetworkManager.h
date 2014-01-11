#pragma once
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

#include "xbmc/network/INetworkManager.h"
#include "threads/Event.h"
#include "AndroidConnection.h"

class IPassphraseStorage;
class CAndroidPassphraseStorage;
class CAndroidNetworkManager : public INetworkManager
{
public:
  CAndroidNetworkManager();
  virtual ~CAndroidNetworkManager();

  virtual bool CanManageConnections();

  virtual ConnectionList GetConnections();

  virtual bool PumpNetworkEvents(INetworkEventsCallback *callback);
  virtual bool ReceiveNetworkEvent(INetworkEventsCallback *callback, const NetworkEventDataPtr data);
  virtual IPassphraseStorage* GetPassphraseStorage();

  static int FindNetworkId(const std::string& ssid);

private:

  CAndroidConnectionPtr m_ethernetConnection;
  CAndroidConnectionPtr m_currentWifi;
  CAndroidConnectionList m_wifiAccessPoints;

  void GetWifiAccessPoints();
  void GetEthernetConnection();
  void GetCurrentWifiConnection();

  CAndroidPassphraseStorage *m_keystorage;
  CEvent m_wifiConnectionEvent;
  CEvent m_ethernetConnectionEvent;
};
