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

#include "AndroidNetworkManager.h"
#include "AndroidConnection.h"
#include "android/jni/ConnectivityManager.h"
#include "android/jni/WifiManager.h"
#include "android/jni/ScanResult.h"
#include "android/jni/List.h"
#include "android/jni/NetworkInfo.h"
#include "android/jni/Context.h"
#include "android/jni/DhcpInfo.h"
#include "android/jni/WifiInfo.h"
#include "android/jni/WifiConfiguration.h"
#include "android/jni/Intent.h"
#include "utils/log.h"
#include "settings/AdvancedSettings.h"
#include "utils/StringUtils.h"
#include "android/activity/XBMCApp.h"
#include "network/linux/AndroidPassphraseStorage.h"

CAndroidNetworkManager::CAndroidNetworkManager()
{
  CLog::Log(LOGDEBUG, "NetworkManager: AndroidNetworkManager created");
  m_keystorage = new CAndroidPassphraseStorage();
}

CAndroidNetworkManager::~CAndroidNetworkManager()
{
  delete m_keystorage;
}

IPassphraseStorage* CAndroidNetworkManager::GetPassphraseStorage()
{
  return m_keystorage;
}

bool CAndroidNetworkManager::CanManageConnections()
{
  return true;
}

ConnectionList CAndroidNetworkManager::GetConnections()
{
  ConnectionList connections;

  if (!m_ethernetConnection.get())
    GetEthernetConnection();
  if (!m_currentWifi.get())
    GetCurrentWifiConnection();
  if (m_wifiAccessPoints.empty())
    GetWifiAccessPoints();


  if(m_ethernetConnection.get())
    connections.push_back(m_ethernetConnection);

  if(m_currentWifi.get() && !m_currentWifi->GetName().empty() && m_currentWifi->GetState() != NETWORK_CONNECTION_STATE_DISCONNECTED)
    connections.push_back(m_currentWifi);

  for(CAndroidConnectionList::iterator i = m_wifiAccessPoints.begin(); i < m_wifiAccessPoints.end(); i++)
  {
    CAndroidConnectionPtr connection(*i);
    if (!connection.get())
      continue;
    if(!connection->GetName().empty() && (!m_currentWifi || (connection->GetName() != m_currentWifi->GetName()) || m_currentWifi->GetState() == NETWORK_CONNECTION_STATE_DISCONNECTED))
      connections.push_back(connection);
  }
  return connections;
}

void  CAndroidNetworkManager::GetEthernetConnection()
{
  CJNIConnectivityManager connectivityManager(CJNIContext::getSystemService("connectivity"));
  if (!connectivityManager)
    return;

  CJNINetworkInfo ethernetInfo = connectivityManager.getNetworkInfo(CJNIConnectivityManager::TYPE_ETHERNET);
  if (!ethernetInfo)
    return;

  if (!m_ethernetConnection.get())
    m_ethernetConnection =  CAndroidConnectionPtr(new CAndroidConnection(ethernetInfo, &m_ethernetConnectionEvent));
  else
    m_ethernetConnection->Update(ethernetInfo);
  m_ethernetConnection->GetDHCPInfo();
}

void CAndroidNetworkManager::GetCurrentWifiConnection()
{
  CJNIWifiManager wifiManager(CJNIContext::getSystemService("wifi"));
  if(!wifiManager || !wifiManager.isWifiEnabled())
    return;

  CJNIWifiInfo wifiInfo = wifiManager.getConnectionInfo();
  if (!wifiInfo)
  {
    m_currentWifi.reset();
    return;
  }

  std::string bssid = wifiInfo.getBSSID();
  for(CAndroidConnectionList::iterator i = m_wifiAccessPoints.begin(); i < m_wifiAccessPoints.end(); i++)
  {
    std::string accessPointBSSID = (*i)->GetBSSID();
    if(bssid == accessPointBSSID)
    {
      m_currentWifi = *i;
      break;
    }
  }

  if (!m_currentWifi.get())
    m_currentWifi = CAndroidConnectionPtr(new CAndroidConnection(wifiInfo, &m_wifiConnectionEvent));
  else
    m_currentWifi->Update(wifiInfo);

  CJNIConnectivityManager connectivityManager(CJNIContext::getSystemService("connectivity"));
  if (connectivityManager)
  {
    CJNINetworkInfo networkInfo = connectivityManager.getNetworkInfo(CJNIConnectivityManager::TYPE_WIFI);
    if (networkInfo)
      m_currentWifi->Update(networkInfo);
    else
    {
      m_currentWifi.reset();
      return;
    }
  }
  m_currentWifi->GetDHCPInfo();
//  if (m_currentWifi->GetState() == NETWORK_CONNECTION_STATE_DISCONNECTED)
//    m_currentWifi.reset();
}

void CAndroidNetworkManager::GetWifiAccessPoints()
{
  CAndroidConnectionList connections;
  CJNIWifiManager wifiManager(CJNIContext::getSystemService("wifi"));
  if(!wifiManager || !wifiManager.isWifiEnabled())
  {
    m_wifiAccessPoints = connections;
    return;
  }

  CJNIList<CJNIScanResult> survey = wifiManager.getScanResults();
  if (!survey)
  {
    m_wifiAccessPoints = connections;
    return;
  }

  int numAccessPoints = survey.size();
  for (int i = 0; i < numAccessPoints; i++)
  {
    CJNIScanResult accessPoint = survey.get(i);
    {
      CAndroidConnectionList::iterator i;
      for(i = m_wifiAccessPoints.begin(); i < m_wifiAccessPoints.end(); i++)
      {
        CAndroidConnectionPtr connection(*i);
        if (!connection.get())
          continue;
        if(accessPoint.SSID == connection->GetName())
        {
          connection->Update(accessPoint);
          connections.push_back(connection);
          break;
        }
      }
      if (i == m_wifiAccessPoints.end())
      {
        connections.push_back(CAndroidConnectionPtr(new CAndroidConnection(accessPoint, &m_wifiConnectionEvent)));
      }
    }
  }
  m_wifiAccessPoints = connections;
}

bool CAndroidNetworkManager::PumpNetworkEvents(INetworkEventsCallback *callback)
{
  // No polling here. See ReceiveNetworkEvent().
  return false;
}

bool CAndroidNetworkManager::ReceiveNetworkEvent(INetworkEventsCallback *callback, const NetworkEventDataPtr data)
{
  // All actions are event-driven. Android broadcasts connection events in the
  // form of subscribed events from XBMCApp. See CXBMCApp::onReceive

  const CJNIIntent intent(*boost::static_pointer_cast<const CJNIIntent>(data));
  std::string action = intent.getAction();
  CLog::Log(LOGDEBUG, "NetworkManager: Received event: %s",action.c_str());

  if (action == CJNIWifiManager::NETWORK_STATE_CHANGED_ACTION)
  {
    GetEthernetConnection();
    GetWifiAccessPoints();
    GetCurrentWifiConnection();
    if(m_currentWifi.get() && m_currentWifi->GetState() == NETWORK_CONNECTION_STATE_CONNECTED)
    {

      // Call the callback before setting the event so that the connection list
      // Is updated before leaving the wifi dialog
      callback->OnConnectionListChange(GetConnections());
      m_wifiConnectionEvent.Set();
    }
    else
      callback->OnConnectionListChange(GetConnections());
  }
  else if(action == CJNIConnectivityManager::CONNECTIVITY_ACTION)
  {
    const CJNINetworkInfo newNetworkInfo = intent.getParcelableExtra(CJNIWifiManager::EXTRA_NETWORK_INFO);
    if (newNetworkInfo)
    {
      int type = newNetworkInfo.getType();
      std::string typeName = newNetworkInfo.getTypeName();
      std::string stateName = newNetworkInfo.getState().name();
      if (type == CJNIConnectivityManager::TYPE_ETHERNET)
      {
        // This one is simple, just pass the new status along to the connection
        GetEthernetConnection();
        if (m_ethernetConnection.get())
        {
          if(m_ethernetConnection->GetState() == NETWORK_CONNECTION_STATE_CONNECTED)
          {
             callback->OnConnectionListChange(GetConnections());
             m_ethernetConnectionEvent.Set();
          }
          else
            callback->OnConnectionListChange(GetConnections());
        }
      }
      else
      {
        GetWifiAccessPoints();
        GetCurrentWifiConnection();
        callback->OnConnectionListChange(GetConnections());
      }
    }
  }
  else if (action == CJNIWifiManager::SCAN_RESULTS_AVAILABLE_ACTION)
  {
    GetWifiAccessPoints();
    callback->OnConnectionListChange(GetConnections());
  }
  else if (action == CJNIWifiManager::SUPPLICANT_STATE_CHANGED_ACTION)
  {
    int error = intent.getIntExtra(CJNIWifiManager::EXTRA_SUPPLICANT_ERROR, 0);
    if (error == 1)
    {
      CLog::Log(LOGDEBUG, "NetworkManager: authentication failed");
      GetCurrentWifiConnection(); 
      callback->OnConnectionListChange(GetConnections());
      m_currentWifi->OnAuthFailed();
      m_wifiConnectionEvent.Set();
    }
  }
  else if (action == CJNIWifiManager::RSSI_CHANGED_ACTION)
  {
    if (!m_currentWifi.get())
      GetCurrentWifiConnection();

    if (m_currentWifi.get())
    {
      int newRSSI = intent.getIntExtra(CJNIWifiManager::EXTRA_NEW_RSSI, 0);
      m_currentWifi->SetStrength(newRSSI);
      callback->OnConnectionChange(m_currentWifi);
    }
  }
  return true;
}

int CAndroidNetworkManager::FindNetworkId(const std::string& ssid)
{
  CJNIWifiManager wifiManager(CJNIContext::getSystemService("wifi"));
  if (!wifiManager)
    return -1;

  CJNIList<CJNIWifiConfiguration> configs = wifiManager.getConfiguredNetworks();

  int numConfigs = configs.size();
  for (int i = 0; i < numConfigs; i++)
  {
    CJNIWifiConfiguration config = configs.get(i);
    if(config && config.getSSID() == "\"" + ssid + "\"")
    {
      return config.getnetworkId();
    }
  }
  return -1;
}
