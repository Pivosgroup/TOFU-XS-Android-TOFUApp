/*
 *      Copyright (C) 2013 Team XBMC
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

#include "WifiManager.h"
#include "DhcpInfo.h"
#include "List.h"
#include "WifiInfo.h"
#include "WifiConfiguration.h"
#include "ScanResult.h"
#include "WifiManagerMulticastLock.h"
#include "jutils/jutils-details.hpp"

using namespace jni;

  int CJNIWifiManager::ERROR_AUTHENTICATING(0);
  std::string CJNIWifiManager::WIFI_STATE_CHANGED_ACTION;
  std::string CJNIWifiManager::EXTRA_WIFI_STATE;
  std::string CJNIWifiManager::EXTRA_PREVIOUS_WIFI_STATE;
  int CJNIWifiManager::WIFI_STATE_DISABLING(0);
  int CJNIWifiManager::WIFI_STATE_DISABLED(0);
  int CJNIWifiManager::WIFI_STATE_ENABLING(0);
  int CJNIWifiManager::WIFI_STATE_ENABLED(0);
  int CJNIWifiManager::WIFI_STATE_UNKNOWN(0);
  std::string CJNIWifiManager::SUPPLICANT_CONNECTION_CHANGE_ACTION;
  std::string CJNIWifiManager::EXTRA_SUPPLICANT_CONNECTED;
  std::string CJNIWifiManager::NETWORK_STATE_CHANGED_ACTION;
  std::string CJNIWifiManager::EXTRA_NETWORK_INFO;
  std::string CJNIWifiManager::EXTRA_BSSID;
  std::string CJNIWifiManager::EXTRA_WIFI_INFO;
  std::string CJNIWifiManager::SUPPLICANT_STATE_CHANGED_ACTION;
  std::string CJNIWifiManager::EXTRA_NEW_STATE;
  std::string CJNIWifiManager::EXTRA_SUPPLICANT_ERROR;
  std::string CJNIWifiManager::SCAN_RESULTS_AVAILABLE_ACTION;
  std::string CJNIWifiManager::RSSI_CHANGED_ACTION;
  std::string CJNIWifiManager::EXTRA_NEW_RSSI;
  std::string CJNIWifiManager::NETWORK_IDS_CHANGED_ACTION;
  std::string CJNIWifiManager::ACTION_PICK_WIFI_NETWORK;
  int CJNIWifiManager::WIFI_MODE_FULL(0);
  int CJNIWifiManager::WIFI_MODE_SCAN_ONLY(0);
  int CJNIWifiManager::WIFI_MODE_FULL_HIGH_PERF(0);

void CJNIWifiManager::PopulateStaticFields()
{
  jhclass clazz  = find_class("android/net/wifi/WifiManager");

  ERROR_AUTHENTICATING = get_static_field<int>(clazz, "ERROR_AUTHENTICATING");
  WIFI_STATE_CHANGED_ACTION = jcast<std::string>(get_static_field<jhstring>(clazz, "WIFI_STATE_CHANGED_ACTION"));
  EXTRA_WIFI_STATE = jcast<std::string>(get_static_field<jhstring>(clazz, "EXTRA_WIFI_STATE"));
  EXTRA_PREVIOUS_WIFI_STATE = jcast<std::string>(get_static_field<jhstring>(clazz, "EXTRA_PREVIOUS_WIFI_STATE"));
  WIFI_STATE_DISABLING = get_static_field<int>(clazz, "WIFI_STATE_DISABLING");
  WIFI_STATE_DISABLED = get_static_field<int>(clazz, "WIFI_STATE_DISABLED");
  WIFI_STATE_ENABLING = get_static_field<int>(clazz, "WIFI_STATE_ENABLING");
  WIFI_STATE_ENABLED = get_static_field<int>(clazz, "WIFI_STATE_ENABLED");
  WIFI_STATE_UNKNOWN =get_static_field<int>(clazz, "WIFI_STATE_UNKNOWN");
  SUPPLICANT_CONNECTION_CHANGE_ACTION = jcast<std::string>(get_static_field<jhstring>(clazz, "SUPPLICANT_CONNECTION_CHANGE_ACTION"));
  EXTRA_SUPPLICANT_CONNECTED = jcast<std::string>(get_static_field<jhstring>(clazz, "EXTRA_SUPPLICANT_CONNECTED"));
  NETWORK_STATE_CHANGED_ACTION = jcast<std::string>(get_static_field<jhstring>(clazz, "NETWORK_STATE_CHANGED_ACTION"));
  EXTRA_NETWORK_INFO = jcast<std::string>(get_static_field<jhstring>(clazz, "EXTRA_NETWORK_INFO"));
  EXTRA_BSSID = jcast<std::string>(get_static_field<jhstring>(clazz, "EXTRA_BSSID"));
  EXTRA_WIFI_INFO = jcast<std::string>(get_static_field<jhstring>(clazz, "EXTRA_WIFI_INFO"));
  SUPPLICANT_STATE_CHANGED_ACTION = jcast<std::string>(get_static_field<jhstring>(clazz, "SUPPLICANT_STATE_CHANGED_ACTION"));
  EXTRA_NEW_STATE = jcast<std::string>(get_static_field<jhstring>(clazz, "EXTRA_NEW_STATE"));
  EXTRA_SUPPLICANT_ERROR = jcast<std::string>(get_static_field<jhstring>(clazz, "EXTRA_SUPPLICANT_ERROR"));
  SCAN_RESULTS_AVAILABLE_ACTION = jcast<std::string>(get_static_field<jhstring>(clazz, "SCAN_RESULTS_AVAILABLE_ACTION"));
  RSSI_CHANGED_ACTION = jcast<std::string>(get_static_field<jhstring>(clazz, "RSSI_CHANGED_ACTION"));
  EXTRA_NEW_RSSI = jcast<std::string>(get_static_field<jhstring>(clazz, "EXTRA_NEW_RSSI"));
  NETWORK_IDS_CHANGED_ACTION = jcast<std::string>(get_static_field<jhstring>(clazz, "NETWORK_IDS_CHANGED_ACTION"));
  ACTION_PICK_WIFI_NETWORK = jcast<std::string>(get_static_field<jhstring>(clazz, "ACTION_PICK_WIFI_NETWORK"));
  WIFI_MODE_FULL = get_static_field<int>(clazz, "WIFI_MODE_FULL");
  WIFI_MODE_SCAN_ONLY = get_static_field<int>(clazz, "WIFI_MODE_SCAN_ONLY");
  WIFI_MODE_FULL_HIGH_PERF = get_static_field<int>(clazz, "WIFI_MODE_FULL_HIGH_PERF");
}

CJNIList<CJNIWifiConfiguration> CJNIWifiManager::getConfiguredNetworks()
{
  return call_method<jhobject>(m_object,
    "getConfiguredNetworks" , "()Ljava/util/List;");
}

int CJNIWifiManager::addNetwork(const CJNIWifiConfiguration &config)
{
    return call_method<int>(m_object,
    "addNetwork" , "(Landroid/net/wifi/WifiConfiguration;)I", config.get_raw());
}

int CJNIWifiManager::updateNetwork(const CJNIWifiConfiguration &config)
{
    return call_method<int>(m_object,
    "updateNetwork" , "(Landroid/net/wifi/WifiConfiguration;)I", config.get_raw());
}

CJNIList<CJNIScanResult> CJNIWifiManager::getScanResults()
{
  return call_method<jhobject>(m_object,
    "getScanResults" , "()Ljava/util/List;");
}

bool CJNIWifiManager::removeNetwork(int netId)
{
  return call_method<jboolean>(m_object,
    "removeNetwork", "(I)Z", netId);
}

bool CJNIWifiManager::enableNetwork(int netID, bool disableOthers)
{
  return call_method<jboolean>(m_object,
    "enableNetwork", "(IZ)Z", netID, disableOthers);
}

bool CJNIWifiManager::disableNetwork(int netID)
{
  return call_method<jboolean>(m_object,
    "disableNetwork", "(I)Z", netID);
}

bool CJNIWifiManager::disconnect()
{
  return call_method<jboolean>(m_object,
    "disconnect", "()Z");
}

bool CJNIWifiManager::reconnect()
{
  return call_method<jboolean>(m_object,
    "reconnect", "()Z");
}

bool CJNIWifiManager::reassociate()
{
  return call_method<jboolean>(m_object,
    "rassociate", "()Z");
}

bool CJNIWifiManager::pingSupplicant()
{
  return call_method<jboolean>(m_object,
    "pingSupplicant", "()Z");
}

bool CJNIWifiManager::startScan()
{
  return call_method<jboolean>(m_object,
    "startScan", "()Z");
}

CJNIWifiInfo CJNIWifiManager::getConnectionInfo()
{
  return call_method<jhobject>(m_object,
    "getConnectionInfo", "()Landroid/net/wifi/WifiInfo;");
}


bool CJNIWifiManager::saveConfiguration()
{
  return call_method<jboolean>(m_object,
    "saveConfiguration", "()Z");
}

CJNIDhcpInfo CJNIWifiManager::getDhcpInfo()
{
  return call_method<jhobject>(m_object,
    "getDhcpInfo", "()Landroid/net/DhcpInfo;");
}

bool CJNIWifiManager::setWifiEnabled(bool enabled)
{
  return call_method<jboolean>(m_object,
    "setWifiEnabled", "(Z)Z", enabled);
}

int CJNIWifiManager::getWifiState()
{
  return call_method<jint>(m_object,
    "getWifiState", "()I");
}

bool CJNIWifiManager::isWifiEnabled()
{
  return call_method<jboolean>(m_object,
    "isWifiEnabled", "()Z");
}

int CJNIWifiManager::calculateSignalLevel(int rssi, int numLevels)
{
  return call_static_method<jint>("android/net/wifi/WifiManager",
    "calculateSignalLevel", "(II)I", rssi, numLevels);
}

int CJNIWifiManager::compareSignalLevel(int rssiA, int rssiB)
{
  return call_static_method<jint>("android/net/wifi/WifiManager",
    "compareSignalLevel", "(II)I", rssiA, rssiB);
}

CJNIWifiManagerMulticastLock CJNIWifiManager::createMulticastLock(const std::string &tag)
{
  return (CJNIWifiManagerMulticastLock)call_method<jhobject>(m_object, "createMulticastLock", "(Ljava/lang/String;)Landroid/net/wifi/WifiManager$MulticastLock;", jcast<jhstring>(tag));
}
