/*
 *      Copyright (C) 2005-2011 Team XBMC
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

#include "AndroidPassphraseStorage.h"
#include "AndroidConnection.h"
#include "AndroidNetworkManager.h"
#include "Application.h"
#include "utils/log.h"
#include "guilib/GUIKeyboardFactory.h"
#include "security/KeyringManager.h"
#include "settings/AdvancedSettings.h"

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
#include "android/jni/BitSet.h"

bool CAndroidPassphraseStorage::HasStoredPassphrase(const std::string &uuid, IConnection *connection)
{
  return CAndroidNetworkManager::FindNetworkId(uuid) > -1;
}

void CAndroidPassphraseStorage::InvalidatePassphrase(const std::string &uuid, IConnection *connection)
{
  int networkId = -1;
    networkId = CAndroidNetworkManager::FindNetworkId(uuid);
  if (networkId > -1)
  {
    CJNIWifiManager wifiManager(CJNIContext::getSystemService("wifi"));
    if (!wifiManager)
      return;

    CLog::Log(LOGDEBUG, "CAndroidPassphraseStorage: Removing configuration for: %s", uuid.c_str());
    wifiManager.removeNetwork(networkId);
  }
}

bool CAndroidPassphraseStorage::GetPassphrase(const std::string &uuid, std::string &passphrase, IConnection *connection)
{
  // We only get here if there was no connection already saved.
  // No need for a lookup, just prompt the user for a key.
  bool result = false;
  CJNIWifiConfiguration config;

  CStdString utf8;
  if (g_advancedSettings.m_showNetworkPassPhrase)
    result = CGUIKeyboardFactory::ShowAndGetInput(utf8, false, 12340);
  else
    result = CGUIKeyboardFactory::ShowAndGetNewPassword(utf8);

  passphrase = utf8;

  return result;
}

void CAndroidPassphraseStorage::StorePassphrase(const std::string &uuid, const std::string &passphrase, IConnection *connection)
{
  // Android's passphrases must be enclosed in quotes!
  if (!connection || connection->GetType() != NETWORK_CONNECTION_TYPE_WIFI)
    return;

  CJNIWifiManager wifiManager(CJNIContext::getSystemService("wifi"));
  if (!wifiManager)
    return;

  CJNIWifiConfiguration config;
  switch(connection->GetEncryption())
  {
    case NETWORK_CONNECTION_ENCRYPTION_NONE:
    {
      CJNIBitSet allowedKeyManagement;
      allowedKeyManagement.set(0);
      config.setallowedKeyManagement(allowedKeyManagement);
      break;
    }
    case NETWORK_CONNECTION_ENCRYPTION_WEP:
    {
      // Android has a bug that causes crashes if all 4 slots aren't filled
      std::vector<std::string> keys(4,"\"" + passphrase + "\"");
      config.setwepTxKeyIndex(0);
      config.setwepKeys(keys);
      CJNIBitSet allowedKeyManagement, allowedAuthAlgorithms;

      allowedKeyManagement.set(0);
      allowedAuthAlgorithms.set(0);
      allowedAuthAlgorithms.set(1);
      
      config.setallowedKeyManagement(allowedKeyManagement);
      config.setallowedAuthAlgorithms(allowedAuthAlgorithms);
      break;
    }
    default: // WPA/WPA2
    {
      config.setpreSharedKey("\"" + passphrase + "\"");
      break;
    }
  }

  config.setSSID("\"" + uuid + "\"");
  config.setpriority(0);
  wifiManager.addNetwork(config);
  wifiManager.saveConfiguration();
}
