#pragma once
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

#include "JNIBase.h"
#include "List.h"

class CJNIDhcpInfo;
class CJNIWifiInfo;
class CJNIScanResult;
class CJNIWifiConfiguration;
class CJNIWifiManagerMulticastLock;

class CJNIWifiManager : public CJNIBase
{
friend class CJNIContext;
public:
  CJNIWifiManager(const jni::jhobject &object) : CJNIBase(object){};

  CJNIList<CJNIWifiConfiguration> getConfiguredNetworks();
  int addNetwork(const CJNIWifiConfiguration &config);
  int updateNetwork(const CJNIWifiConfiguration &config);
  bool removeNetwork(int);
  bool enableNetwork(int, bool);
  bool disableNetwork(int);
  bool disconnect();
  bool reconnect();
  bool reassociate();
  bool pingSupplicant();
  bool startScan();
  CJNIWifiInfo getConnectionInfo();
  CJNIList<CJNIScanResult> getScanResults();
  bool saveConfiguration();
  CJNIDhcpInfo getDhcpInfo();
  bool setWifiEnabled(bool);
  int  getWifiState();
  bool isWifiEnabled();
  static int calculateSignalLevel(int, int);
  static int compareSignalLevel(int, int);
  CJNIWifiManagerMulticastLock createMulticastLock(const std::string &tag);

  static void PopulateStaticFields();
  static int ERROR_AUTHENTICATING;
  static std::string WIFI_STATE_CHANGED_ACTION;
  static std::string EXTRA_WIFI_STATE;
  static std::string EXTRA_PREVIOUS_WIFI_STATE;
  static int WIFI_STATE_DISABLING;
  static int WIFI_STATE_DISABLED;
  static int WIFI_STATE_ENABLING;
  static int WIFI_STATE_ENABLED;
  static int WIFI_STATE_UNKNOWN;
  static std::string SUPPLICANT_CONNECTION_CHANGE_ACTION;
  static std::string EXTRA_SUPPLICANT_CONNECTED;
  static std::string NETWORK_STATE_CHANGED_ACTION;
  static std::string EXTRA_NETWORK_INFO;
  static std::string EXTRA_BSSID;
  static std::string EXTRA_WIFI_INFO;
  static std::string SUPPLICANT_STATE_CHANGED_ACTION;
  static std::string EXTRA_NEW_STATE;
  static std::string EXTRA_SUPPLICANT_ERROR;
  static std::string SCAN_RESULTS_AVAILABLE_ACTION;
  static std::string RSSI_CHANGED_ACTION;
  static std::string EXTRA_NEW_RSSI;
  static std::string NETWORK_IDS_CHANGED_ACTION;
  static std::string ACTION_PICK_WIFI_NETWORK;
  static int WIFI_MODE_FULL;
  static int WIFI_MODE_SCAN_ONLY;
  static int WIFI_MODE_FULL_HIGH_PERF;

private:
  CJNIWifiManager();
};
