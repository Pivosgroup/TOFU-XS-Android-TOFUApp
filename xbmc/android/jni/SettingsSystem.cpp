/*
 *      Copyright (C) 2013 Team XBMC
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
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "SettingsSystem.h"
#include "ContentResolver.h"
#include "jutils/jutils-details.hpp"

using namespace jni;

const char* CJNISettingsSystem::m_classname("android.provider.Settings$System");

std::string CJNISettingsSystem::getString(const CJNIContentResolver& resolver, const std::string &name)
{
  return jcast<std::string>(call_static_method<jhstring>(m_classname,
    "getString", "(Landroid/content/ContentResolver;Ljava/lang/String;)Ljava/lang/String;",
    resolver.get_raw(), jcast<jhstring>(name)));
}

bool CJNISettingsSystem::putString(const CJNIContentResolver& resolver, const std::string &name, const std::string &value)
{
  return call_static_method<jboolean>(m_classname,
    "putString", "(Landroid/content/ContentResolver;Ljava/lang/String;Ljava/lang/String;)Z",
    resolver.get_raw(), jcast<jhstring>(name), jcast<jhstring>(value));
}

CJNIURI CJNISettingsSystem::getUriFor(const std::string &name)
{
  return call_static_method<jhobject>(m_classname,
    "getUriFor", "(Ljava/lang/String;)Landroid/net/Uri;",
    jcast<jhstring>(name));
}

int CJNISettingsSystem::getInt(const CJNIContentResolver& resolver, const std::string &name, int def)
{
  return call_static_method<int>(m_classname,
    "getInt", "(Landroid/content/ContentResolver;Ljava/lang/String;I)I",
    resolver.get_raw(), jcast<jhstring>(name), def);
}

int CJNISettingsSystem::getInt(const CJNIContentResolver& resolver, const std::string &name)
{
  return call_static_method<int>(m_classname,
    "getInt", "(Landroid/content/ContentResolver;Ljava/lang/String;)I",
    resolver.get_raw(), jcast<jhstring>(name));
}

bool CJNISettingsSystem::putInt(const CJNIContentResolver& resolver, const std::string &name, int def)
{
  return call_static_method<jboolean>(m_classname,
    "putInt", "(Landroid/content/ContentResolver;Ljava/lang/String;I)Z",
    resolver.get_raw(), jcast<jhstring>(name), def);
}

int64_t CJNISettingsSystem::getLong(const CJNIContentResolver& resolver, const std::string &name, int64_t def)
{
  return call_static_method<jlong>(m_classname,
    "getLong", "(Landroid/content/ContentResolver;Ljava/lang/String;J)J",
    resolver.get_raw(), jcast<jhstring>(name), def);
}

int64_t CJNISettingsSystem::getLong(const CJNIContentResolver& resolver, const std::string &name)
{
  return call_static_method<jlong>(m_classname,
    "getLong", "(Landroid/content/ContentResolver;Ljava/lang/String;)J",
    resolver.get_raw(), jcast<jhstring>(name));
}

bool CJNISettingsSystem::putLong(const CJNIContentResolver& resolver, const std::string &name, int64_t def)
{
  return call_static_method<jboolean>(m_classname,
    "putLong", "(Landroid/content/ContentResolver;Ljava/lang/String;J)Z",
    resolver.get_raw(), jcast<jhstring>(name), def);
}

float CJNISettingsSystem::getFloat(const CJNIContentResolver& resolver, const std::string &name, float def)
{
  return call_static_method<float>(m_classname,
    "getFloat", "(Landroid/content/ContentResolver;Ljava/lang/String;F)F",
    resolver.get_raw(), jcast<jhstring>(name), def);
}

float CJNISettingsSystem::getFloat(const CJNIContentResolver& resolver, const std::string &name)
{
  return call_static_method<float>(m_classname,
    "getFloat", "(Landroid/content/ContentResolver;Ljava/lang/String;)F",
    resolver.get_raw(), jcast<jhstring>(name));
}

bool CJNISettingsSystem::putFloat(const CJNIContentResolver& resolver, const std::string &name, float def)
{
  return call_static_method<jboolean>(m_classname,
    "putFloat", "(Landroid/content/ContentResolver;Ljava/lang/String;F)Z",
    resolver.get_raw(), jcast<jhstring>(name), def);
}

/*
void getConfiguration(const CJNIContentResolver&, android.content.res.Configuration)
{
}

bool putConfiguration(const CJNIContentResolver&, android.content.res.Configuration)
{
}
*/

bool CJNISettingsSystem::getShowGTalkServiceStatus(const CJNIContentResolver& resolver)
{
  return call_static_method<jboolean>(m_classname,
    "getShowGTalkServiceStatus", "(Landroid/content/ContentResolver;)Z",
    resolver.get_raw());
}

void CJNISettingsSystem::setShowGTalkServiceStatus(const CJNIContentResolver& resolver, bool flag)
{
  call_static_method<void>(m_classname,
    "getShowGTalkServiceStatus", "(Landroid/content/ContentResolver;Z)Z",
    resolver.get_raw(), flag);
}
