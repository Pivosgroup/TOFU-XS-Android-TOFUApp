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


#include "IConnection.h"

typedef boost::shared_ptr<const void> NetworkEventDataPtr;

/*!
 \ingroup network
 \brief Callback interface for network events.

 Used by subsystems of the network manager to receive events about the subsystem.
 It will be called relatively often from application thread to allow thread safe updates
 of the network states.

 \sa INetworkManager
 */
class INetworkEventsCallback
{
public:
  virtual ~INetworkEventsCallback() { }

  virtual void OnConnectionStateChange(ConnectionState state) = 0;
  virtual void OnConnectionChange(CConnectionPtr connection) = 0;
  virtual void OnConnectionListChange(ConnectionList list) = 0;
};


/*!
 \ingroup network
 \brief Interface for the network subsystems context

 Used by subsystems of the network manager to receive events about the subsystem.
 It will be called relatively often from application thread to allow thread safe updates
 of the network states.

 \sa INetworkManager
 */
class INetworkManager
{
public:
  virtual ~INetworkManager() { }

  virtual bool CanManageConnections() = 0;

  virtual      ConnectionList GetConnections() = 0;

  virtual bool PumpNetworkEvents(INetworkEventsCallback *callback) = 0;

  virtual bool ReceiveNetworkEvent(INetworkEventsCallback *callback, const NetworkEventDataPtr data) {return false;};

  virtual IPassphraseStorage* GetPassphraseStorage() = 0;
};
