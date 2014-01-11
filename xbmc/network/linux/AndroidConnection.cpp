/*
 *      Copyright (C) 2011-2012 Team XBMC
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

#include "AndroidConnection.h"
#include "AndroidNetworkManager.h"
#include "Application.h"
#include "guilib/GUIKeyboardFactory.h"
#include "network/IPassphraseStorage.h"
#include "security/KeyringManager.h"
#include "settings/AdvancedSettings.h"
#include "sys/system_properties.h"
#include "utils/StringUtils.h"
#include "utils/log.h"

#include "android/jni/Intent.h"
#include "android/jni/NetworkInfo.h"
#include "android/jni/WifiInfo.h"
#include "android/jni/DhcpInfo.h"
#include "android/jni/ScanResult.h"
#include "android/jni/ConnectivityManager.h"
#include "android/jni/WifiManager.h"
#include "android/jni/WifiConfiguration.h"
#include "android/jni/Context.h"
#include "android/jni/ContentResolver.h"
#include "android/jni/SettingsSystem.h"
#include "android/jni/SettingsSecure.h"
#include "android/jni/NetworkInterface.h"

#include <arpa/inet.h>

static void StripDoubleQuotes(std::string &str)
{
  std::string::iterator it = str.begin();
  while(it != str.end())
  {
    if (*it == '\"')
      it = str.erase(it);

    if (it != str.end())
      ++it;
  }
}

// Non-Wifi interfaces
CAndroidConnection::CAndroidConnection(const CJNINetworkInfo &networkInfo, CEvent *connectionEvent)
: m_name(networkInfo.getTypeName())
, m_macaddress("00:00:00:00:00:00")
, m_rawmacaddress("")
, m_state(NETWORK_CONNECTION_STATE_DISCONNECTED)
, m_method(IP_CONFIG_DHCP)
, m_encryption(NETWORK_CONNECTION_ENCRYPTION_NONE)
, m_signal(0)
, m_speed(0)
, m_networkId(-1)
, m_connectionEvent(connectionEvent)
, m_authFailed(false)
{
  if(networkInfo.getType() == CJNIConnectivityManager::TYPE_ETHERNET)
  {
    CJNINetworkInterface eth0 = CJNINetworkInterface::getByName("eth0");
    if (eth0)
    {
      std::vector<char> chars = eth0.getHardwareAddress();
      std::string macaddress;

      for(std::vector<char>::iterator i = chars.begin(); i != chars.end(); i++)
      {
        char hexstr[2];
        sprintf(hexstr, "%02x", *i);
        macaddress += hexstr;
        macaddress += ":";
        m_rawmacaddress += *i;
      }
      m_macaddress = macaddress.substr(0,17);
      CLog::Log(LOGDEBUG, "CAndroidConnection: Ethernet mac address: %s",GetMacAddress().c_str());
    }
  }
  Update(networkInfo);
}

// Current Wifi Connection
CAndroidConnection::CAndroidConnection(const CJNIWifiInfo &wifiInfo, CEvent *connectionEvent)
: m_type(CJNIConnectivityManager::TYPE_WIFI)
, m_state(NETWORK_CONNECTION_STATE_DISCONNECTED)
, m_method(IP_CONFIG_DHCP)
, m_encryption(NETWORK_CONNECTION_ENCRYPTION_UNKNOWN)
, m_signal(0)
, m_speed(0)
, m_networkId(-1)
, m_connectionEvent(connectionEvent)
, m_authFailed(false)
{
  Update(wifiInfo);
}

// Available access points
CAndroidConnection::CAndroidConnection(const CJNIScanResult &scanResult, CEvent *connectionEvent)
: m_type(CJNIConnectivityManager::TYPE_WIFI)
, m_state(NETWORK_CONNECTION_STATE_DISCONNECTED)
, m_method(IP_CONFIG_DHCP)
, m_encryption(NETWORK_CONNECTION_ENCRYPTION_UNKNOWN)
, m_signal(0)
, m_speed(0)
, m_networkId(-1)
, m_connectionEvent(connectionEvent)
, m_authFailed(false)
{
  Update(scanResult);
}

CAndroidConnection::CAndroidConnection::~CAndroidConnection()
{
}

void CAndroidConnection::Update(const CJNINetworkInfo &networkInfo)
{
  // We've received a state-change. If connected, set the the event to break
  // out of Connect() successfully.
  m_type= networkInfo.getType();
  m_state = GetState(networkInfo.getState().name());
  CLog::Log(LOGDEBUG, "CAndroidConnection: Setting state to: %s.", networkInfo.getState().name().c_str());
}

void CAndroidConnection::Update(const CJNIWifiInfo &wifiInfo)
{
  m_name = wifiInfo.getSSID();
  // If the SSID can be decoded as UTF-8, it will be returned
  // surrounded by double quotation marks. Strip them.
  StripDoubleQuotes(m_name);
  m_signal = CJNIWifiManager::calculateSignalLevel(wifiInfo.getRssi(), 100);
  m_address = GetAddress(wifiInfo.getIpAddress());
  m_networkId = wifiInfo.getNetworkId();
  m_macaddress = wifiInfo.getMacAddress();
}

void CAndroidConnection::Update(const CJNIScanResult &scanResult)
{
  m_name = scanResult.SSID;
  m_bssid = scanResult.BSSID;
  m_signal = CJNIWifiManager::calculateSignalLevel(scanResult.level, 100);
  m_encryption = GetEncryption(scanResult.capabilities);
  CLog::Log(LOGDEBUG, "CAndroidConnection: Scanresult capabilities: %s. Type: %i", scanResult.capabilities.c_str(), m_encryption);
}

std::string CAndroidConnection::GetName() const
{
  return m_name;
}

std::string CAndroidConnection::GetUUID() const
{
  return m_name;
}

std::string CAndroidConnection::GetAddress() const
{
  return m_address;
}

std::string CAndroidConnection::GetNetmask() const
{
  return m_netmask;
}

std::string CAndroidConnection::GetGateway() const
{
  return m_gateway;
}

std::string CAndroidConnection::GetNameServer() const
{
  return m_nameserver;
}

std::string CAndroidConnection::GetMacAddress() const
{
  std::string bigMac = m_macaddress;
  StringUtils::ToUpper(bigMac);
  return bigMac; // I said no onion.
}

void CAndroidConnection::GetMacAddressRaw(char rawMac[6]) const
{
  memcpy(rawMac, m_rawmacaddress.c_str(), 6);
}

std::string CAndroidConnection::GetInterfaceName() const
{
  return m_interface;
}


ConnectionType CAndroidConnection::GetType() const
{
  return GetType(m_type);
}

ConnectionState CAndroidConnection::GetState() const
{
  return m_state;
}

unsigned int CAndroidConnection::GetSpeed() const
{
  return m_speed;
}

IPConfigMethod CAndroidConnection::GetMethod() const
{
  return m_method;
}

unsigned int CAndroidConnection::GetStrength() const
{
  return m_signal;
}

EncryptionType CAndroidConnection::GetEncryption() const
{
  return m_encryption;
}

bool CAndroidConnection::Connect(IPassphraseStorage *storage, const CIPConfig &ipconfig)
{
  CLog::Log(LOGDEBUG, "Trying to connect to: %s", m_name.c_str());
  m_state = NETWORK_CONNECTION_STATE_CONNECTING;

  if (m_connectionEvent)
    m_connectionEvent->Reset();

  if (m_type == CJNIConnectivityManager::TYPE_WIFI)
  {
    if (!ConnectWifi(storage, ipconfig))
      return false;
  }

  // Give the requested network type (wifi/ethernet/etc) top priority
  CJNIConnectivityManager connectivityManager = CJNIContext::getSystemService("connectivity");
  if (connectivityManager)
    connectivityManager.setNetworkPreference(m_type);

  if (m_type == CJNIConnectivityManager::TYPE_ETHERNET)
  {
    // Disconnect wifi to force an ethernet connection
    CJNIWifiManager wifiManager(CJNIContext::getSystemService("wifi"));
    if (wifiManager)
      wifiManager.disconnect();
  }

  // This function must block until connected. If we're not already, block
  // until we get the connected event or timeout.

  if (m_state == NETWORK_CONNECTION_STATE_CONNECTED)
    return true;

  // Give it 30 seconds to try to connect. We must block here, otherwise the
  // the networkmanager thinks we're done
  if(m_connectionEvent)
    m_connectionEvent->WaitMSec(30000);

  if(m_authFailed)
  {
    m_authFailed = false;
    storage->InvalidatePassphrase(m_name, this);
    return false;
  }

  return m_state == NETWORK_CONNECTION_STATE_CONNECTED;
}

void CAndroidConnection::Disconnect()
{
  if (m_state == NETWORK_CONNECTION_STATE_CONNECTED)
  {
    CLog::Log(LOGDEBUG, "Disconnecting: id: %i", m_networkId);
    if (m_type == CJNIConnectivityManager::TYPE_WIFI)
    {
      CJNIWifiManager wifiManager(CJNIContext::getSystemService("wifi"));
      if (wifiManager)
      {
        if (wifiManager.disconnect())
          m_state = NETWORK_CONNECTION_STATE_DISCONNECTED;
      }
    }
    else if(m_type == CJNIConnectivityManager::TYPE_ETHERNET)
    {
      CJNIConnectivityManager connectivityManager = CJNIContext::getSystemService("connectivity");
      if (connectivityManager)
        connectivityManager.setNetworkPreference(CJNIConnectivityManager::TYPE_WIFI);
    }
  }

}

//-----------------------------------------------------------------------

bool CAndroidConnection::ConnectWifi(IPassphraseStorage *storage, const CIPConfig &ipconfig)
{

  // See if we have a stored network that matches this ssid.
  if (m_networkId < 0)
  {
    int networkId = CAndroidNetworkManager::FindNetworkId(m_name);
    if (networkId >= 0)
      m_networkId = networkId;
  }

  // If we already have a networkId, the connection data is already stored
  // and nothing else should be required if the info is valid. Otherwise if
  // the network is encrypted we prompt the user for the key.
  // StorePassphrase is responsible for creating the networkId for encrypted
  // networks. If unencrypted, it should be generated here by adding a new
  // network config.
  if (m_networkId < 0)
  {
    if (GetEncryption() == NETWORK_CONNECTION_ENCRYPTION_NONE)
    {
      CJNIWifiManager wifiManager(CJNIContext::getSystemService("wifi"));
      if (!wifiManager)
        return false;

      CLog::Log(LOGDEBUG, "CAndroidConnection:: Attempting to configure unencrypted SSID: %s",m_name.c_str());
      CJNIWifiConfiguration config;
      config.setSSID("\"" + m_name + "\"");
      config.setpriority(0);
      wifiManager.addNetwork(config);
      wifiManager.saveConfiguration();
    }
    else if(storage)
    {
      CLog::Log(LOGDEBUG, "CAndroidConnection: Encryption key required for SSID:: %s",m_name.c_str());
      std::string passphrase;
      if (storage->GetPassphrase(m_name, passphrase, this))
        storage->StorePassphrase(m_name, passphrase, this);
    }

    // A passphrase has been entered if necessary. The config has been created,
    // added, and saved. We should now have a networkId. Iterate through the
    // networks to see if there's a networkId that matches our SSID.
    int networkId = CAndroidNetworkManager::FindNetworkId(m_name);
    if (networkId > 0)
      m_networkId = networkId;

  }

  // Now that we nave a networkId we can try to connect to it
  if (m_networkId >= 0)
  {
    CJNIWifiManager wifiManager(CJNIContext::getSystemService("wifi"));
    if (!wifiManager)
      return false;

    CLog::Log(LOGDEBUG, "Enabling: id: %i", m_networkId);
    wifiManager.enableNetwork(m_networkId, true);
  }
  return m_networkId >= 0;
}

EncryptionType CAndroidConnection::GetEncryption(const std::string &capabilities) const
{
  if (capabilities.find("WPA2") != std::string::npos)
     return NETWORK_CONNECTION_ENCRYPTION_WPA2;
  else if(capabilities.find("WPA") != std::string::npos)
     return NETWORK_CONNECTION_ENCRYPTION_WPA;
  else if(capabilities.find("WEP") != std::string::npos)
     return NETWORK_CONNECTION_ENCRYPTION_WEP;
  else
    return NETWORK_CONNECTION_ENCRYPTION_NONE;
}

ConnectionState CAndroidConnection::GetState(const std::string &state) const
{
  if (state == "CONNECTED")
    return NETWORK_CONNECTION_STATE_CONNECTED;
  else if (state == "CONNECTING"           || \
           state == "AUTHENTICATING"       || \
           state == "OBTAINING_IPADDR")
    return NETWORK_CONNECTION_STATE_CONNECTING;
  else if (state == "BLOCKED"              || \
             state == "FAILED")
    return NETWORK_CONNECTION_STATE_FAILURE;

  return NETWORK_CONNECTION_STATE_DISCONNECTED;
}

ConnectionType CAndroidConnection::GetType(int type) const
{
  if (type == CJNIConnectivityManager::TYPE_ETHERNET)
    return NETWORK_CONNECTION_TYPE_WIRED;
  else if (type == CJNIConnectivityManager::TYPE_WIFI)
    return NETWORK_CONNECTION_TYPE_WIFI;
  return NETWORK_CONNECTION_TYPE_UNKNOWN;
}

std::string CAndroidConnection::GetAddress(int ip)
{
  char str[INET_ADDRSTRLEN];
  inet_ntop(AF_INET, &ip, str, INET_ADDRSTRLEN);
  return str;
}

void CAndroidConnection::SetStrength(const int strength)
{
  m_signal = CJNIWifiManager::calculateSignalLevel(strength, 100);
}

std::string CAndroidConnection::GetBSSID() const
{
  return m_bssid;
}

void CAndroidConnection::OnAuthFailed()
{
  m_authFailed = true;
}

void CAndroidConnection::GetDHCPInfo()
{
  if (!m_state == NETWORK_CONNECTION_STATE_CONNECTED)
    return;
  if (m_type == CJNIConnectivityManager::TYPE_WIFI)
  {
    CJNIWifiManager wifiManager(CJNIContext::getSystemService("wifi"));
    if (!wifiManager)
      return;
    CJNIDhcpInfo dhcp = wifiManager.getDhcpInfo();
    if (!dhcp)
      return;

    m_address = GetAddress(dhcp.ipAddress);
    m_gateway = GetAddress(dhcp.gateway);
    m_nameserver = GetAddress(dhcp.dns1);
    m_netmask = GetAddress(dhcp.netmask);
  }
  else if (m_type == CJNIConnectivityManager::TYPE_ETHERNET)
  {
    // There is no real documented way to get this info reliably (besides
    // perhaps querying NetworkInterface with the ethernet type and hoping
    // that it's registered there). So we fall back to the patches that most
    // vendors seem to be using for ethernet. If the secure config ETH_MODE
    // is set, use that for static ip info. Otherwise try to grab from system
    // properties.
    // TODO: Once a standard interface for this appears in Android, this should
    // be deprecated.
    CJNIContentResolver resolver = CJNIContext::getContentResolver();
    if (resolver && CJNISettingsSecure::getString(resolver, "eth_mode") == "manual")
    {
      m_address = CJNISettingsSecure::getString(resolver, "eth_ip");
      m_gateway = CJNISettingsSecure::getString(resolver, "eth_route");
      m_netmask = CJNISettingsSecure::getString(resolver, "eth_mask");
      m_nameserver = CJNISettingsSecure::getString(resolver, "eth_dns");
    }
    else
    {
      char value[PROP_VALUE_MAX];
      if(__system_property_get("dhcp.eth0.ipaddress", value))
        m_address = value;
      if(__system_property_get("dhcp.eth0.gateway", value))
        m_gateway = value;
      if(__system_property_get("dhcp.eth0.dns1", value))
        m_nameserver = value;
      if(__system_property_get("dhcp.eth0.mask", value))
        m_netmask = value;
    }
  }
}
