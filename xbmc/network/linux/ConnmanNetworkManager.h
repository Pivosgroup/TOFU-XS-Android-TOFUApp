#pragma once
/*
 *      Copyright (C) 2005-2013 Team XBMC
 *      http://xbmc.org
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
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */


#include "system.h"
#ifdef HAS_DBUS
#include "network/INetworkManager.h"
#include "utils/Variant.h"
#include <dbus/dbus.h>

class CConnmanNetworkManager : public INetworkManager
{
public:
  CConnmanNetworkManager();
  virtual ~CConnmanNetworkManager();


  virtual bool CanManageConnections();

  virtual ConnectionList GetConnections();

  virtual bool PumpNetworkEvents(INetworkEventsCallback *callback);

  static bool HasConnman();
private:
  void UpdateNetworkManager();

  ConnectionList m_connections;
  CVariant m_properties;
  DBusConnection *m_connection;
  DBusError m_error;
};
#endif
