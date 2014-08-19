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


#include "NullNetworkManager.h"

#include <string.h>

//-----------------------------------------------------------------------
//-----------------------------------------------------------------------
CNullConnection::~CNullConnection()
{
}

std::string CNullConnection::GetName() const
{
  return "no connection";
}

std::string CNullConnection::GetUUID() const
{
  return "";
}


std::string CNullConnection::GetAddress() const
{
  return "127.0.0.1";
}

std::string CNullConnection::GetNetmask() const
{
  return "255.255.255.0";
}

std::string CNullConnection::GetGateway() const
{
  return "127.0.0.1";
}

std::string CNullConnection::GetNameServer() const
{
  return "127.0.0.1";
}

std::string CNullConnection::GetMacAddress() const
{
  return "00:00:00:00:00:00";
}

void CNullConnection::GetMacAddressRaw(char rawMac[6]) const
{
  memset(rawMac, 0x00, 6);
}

std::string CNullConnection::GetInterfaceName() const
{
  return "no interface";
}

ConnectionType CNullConnection::GetType() const
{
  return NETWORK_CONNECTION_TYPE_UNKNOWN;
}

unsigned int CNullConnection::GetSpeed() const
{
  return 100;
}

ConnectionState CNullConnection::GetState() const
{
  return NETWORK_CONNECTION_STATE_DISCONNECTED;
}

IPConfigMethod CNullConnection::GetMethod() const
{
  return IP_CONFIG_DISABLED;
}

unsigned int CNullConnection::GetStrength() const
{
  return 100;
}

EncryptionType CNullConnection::GetEncryption() const
{
  return NETWORK_CONNECTION_ENCRYPTION_NONE;
}

bool CNullConnection::Connect(IPassphraseStorage *storage, const CIPConfig &ipconfig)
{
  return false;
}

void CNullConnection::Disconnect()
{

}

//-----------------------------------------------------------------------
//-----------------------------------------------------------------------
CNullNetworkManager::~CNullNetworkManager()
{
}

bool CNullNetworkManager::CanManageConnections()
{
  return false;
}

ConnectionList CNullNetworkManager::GetConnections()
{
  ConnectionList list;
  list.push_back(CConnectionPtr(new CNullConnection()));
  return list;
}

bool CNullNetworkManager::PumpNetworkEvents(INetworkEventsCallback *callback)
{
  return true;
}

IPassphraseStorage* CNullNetworkManager::GetPassphraseStorage()
{
  return this;
}

//-----------------------------------------------------------------------
//-----------------------------------------------------------------------

bool CNullNetworkManager::HasStoredPassphrase(const std::string &uuid, IConnection* connection)
{
  return false;
}

void CNullNetworkManager::InvalidatePassphrase(const std::string &uuid, IConnection *connection)
{
}

bool CNullNetworkManager::GetPassphrase(const std::string &uuid, std::string &passphrase, IConnection *connection)
{
  return false;
}

void CNullNetworkManager::StorePassphrase(const std::string &uuid, const std::string &passphrase, IConnection *connection)
{
}
