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


#include "xbmc/network/IConnection.h"

int  PosixParseHex(char *str, unsigned char *addr);
bool IsWireless(int socket, const char *interface);
bool PosixCheckInterfaceUp(const std::string &interface);
std::string PosixGetDefaultGateway(const std::string &interface);
std::string PosixGetMacAddress(const std::string& interfaceName);
void PosixGetMacAddressRaw(const std::string& interfaceName, char rawMac[6]);

//-----------------------------------------------------------------------
//-----------------------------------------------------------------------
class CPosixConnection : public IConnection
{
public:
  CPosixConnection(bool managed,
    int socket, const char *interface, const char *macaddress,
    const char *essid, ConnectionType type, EncryptionType encryption, int signal);
  virtual ~CPosixConnection();

  virtual std::string     GetName()       const;
  virtual std::string     GetUUID()       const;
  virtual std::string     GetAddress()    const;
  virtual std::string     GetNetmask()    const;
  virtual std::string     GetGateway()    const;
  virtual std::string     GetNameServer() const;
  virtual std::string     GetMacAddress() const;
  virtual void            GetMacAddressRaw(char rawMac[6]) const;
  virtual std::string     GetInterfaceName() const;

  virtual ConnectionType  GetType()       const;
  virtual ConnectionState GetState()      const;
  virtual unsigned int    GetSpeed()      const;
  virtual IPConfigMethod  GetMethod()     const;
  virtual unsigned int    GetStrength()   const;
  virtual EncryptionType  GetEncryption() const;

  virtual bool            Connect(IPassphraseStorage *storage, const CIPConfig &ipconfig);
  virtual void            Disconnect(){};
  bool PumpNetworkEvents();

  void SetCanManage(bool can_manange);
private:
  bool DoConnection(const CIPConfig &ipconfig, std::string passphrase);

  bool            m_managed;

  std::string     m_essid;
  std::string     m_address;
  std::string     m_netmask;
  std::string     m_gateway;
  std::string     m_macaddress;

  ConnectionType  m_type;
  ConnectionState m_state;
  IPConfigMethod  m_method;
  EncryptionType  m_encryption;

  int             m_signal;
  int             m_socket;
  std::string     m_interface;
};
