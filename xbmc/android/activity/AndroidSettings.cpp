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

#include "AndroidSettings.h"
#include "settings/lib/Setting.h"
#include "utils/log.h"
#include "xbmc/android/activity/XBMCApp.h"

CAndroidSettings::CAndroidSettings()
{
}

CAndroidSettings::~CAndroidSettings()
{
}

CAndroidSettings& CAndroidSettings::Get()
{
  static CAndroidSettings sAndroidSettings;
  return sAndroidSettings;
}

void CAndroidSettings::OnSettingsLoaded()
{
}

void CAndroidSettings::OnSettingsUnloaded()
{
}

void CAndroidSettings::OnSettingAction(const CSetting *setting)
{
  if (setting == NULL)
    return;

  const std::string &settingId = setting->GetId();
  if (settingId == "android.settings")
  {
    // android.settings acts as a button
    // to launch the Android setttings App.
    std::string packageName = "com.android.settings";
    std::string componentName = "com.android.settings.Settings";
    CLog::Log(LOGDEBUG, "CAndroidSettings::OnSettingAction, Trying %s@%s",
      packageName.c_str(), componentName.c_str());
    CXBMCApp::StartActivityByComponent(packageName, componentName);
  }
}
