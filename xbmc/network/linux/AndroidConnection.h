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

#include "xbmc/network/IConnection.h"
#include "threads/Event.h"

class CJNINetworkInfo;
class CJNIScanResult;
class CJNIWifiInfo;
class CJNIWifiConfiguration;
class CJNIIntent;
class CAndroidConnection : public IConnection
{
public:
  CAndroidConnection(const CJNINetworkInfo &networkInfo, CEvent *connectionEvent);
  CAndroidConnection(const CJNIWifiInfo &wifiInfo, CEvent *connectionEvent);
  CAndroidConnection(const CJNIScanResult &scanResult, CEvent *connectionEvent);
  virtual ~CAndroidConnection();

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
  virtual void            Disconnect();

  std::string             GetBSSID() const;
  void                    SetCanManage(bool can_manange);

  void                    SetStrength(const int strength);
  void                    Update(const CJNINetworkInfo &networkInfo);
  void                    Update(const CJNIWifiInfo &wifiInfo);
  void                    Update(const CJNIScanResult &scanResult);

  void                    OnAuthFailed();
  void                    GetDHCPInfo();
private:
  bool                    ConnectWifi(IPassphraseStorage *storage, const CIPConfig &ipconfig);
  EncryptionType          GetEncryption(const std::string &capabilities) const;
  ConnectionState         GetState(const std::string &state) const ;
  ConnectionType          GetType(int type) const;
  std::string             GetAddress(int ip);

  bool                    m_managed;

  std::string             m_name;
  std::string             m_address;
  std::string             m_netmask;
  std::string             m_gateway;
  std::string             m_macaddress;
  std::string             m_nameserver;
  std::string             m_rawmacaddress;

  int  m_type;
  ConnectionState         m_state;
  IPConfigMethod          m_method;
  EncryptionType          m_encryption;

  int                     m_signal;
  int                     m_speed;
  std::string             m_interface;

  int                     m_networkId;
  CEvent*                 m_connectionEvent;
  std::string             m_bssid;
  bool                    m_authFailed;
};

typedef boost::shared_ptr<CAndroidConnection> CAndroidConnectionPtr;
typedef std::vector<CAndroidConnectionPtr> CAndroidConnectionList;
