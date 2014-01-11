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

#include "KeyringStorage.h"
#include "Application.h"
#include "utils/log.h"
#include "guilib/GUIKeyboardFactory.h"
#include "security/KeyringManager.h"
#include "settings/AdvancedSettings.h"
#include "settings/Settings.h"

bool CKeyringStorage::HasStoredPassphrase(const std::string &uuid, IConnection *connection)
{
  CVariant secret;
  return g_application.getKeyringManager().FindSecret("network", uuid, secret);
}

void CKeyringStorage::InvalidatePassphrase(const std::string &uuid, IConnection *connection)
{
  g_application.getKeyringManager().EraseSecret("network", uuid);
  CSettings::Get().SetString("network.passphrase", "");
}

bool CKeyringStorage::GetPassphrase(const std::string &uuid, std::string &passphrase, IConnection *connection)
{
  passphrase = CSettings::Get().GetString("network.passphrase");
  if (passphrase.size() > 0)
    return true;
  /*
  CVariant secret;
  if (g_application.getKeyringManager().FindSecret("network", uuid, secret) && secret.isString())
  {
    passphrase = secret.asString();
    return true;
  }
  */
  else
  {
    bool result;
    CStdString utf8;
    if (g_advancedSettings.m_showNetworkPassPhrase)
      result = CGUIKeyboardFactory::ShowAndGetInput(utf8, false, 12340);
    else
      result = CGUIKeyboardFactory::ShowAndGetNewPassword(utf8);

    passphrase = utf8;
    StorePassphrase(uuid, passphrase);
    return result;
  }
}

void CKeyringStorage::StorePassphrase(const std::string &uuid, const std::string &passphrase, IConnection *connection)
{
  g_application.getKeyringManager().StoreSecret("network", uuid, CVariant(passphrase));
  // hack until we get keyring storage working
  CSettings::Get().SetString("network.passphrase", passphrase.c_str());
}

