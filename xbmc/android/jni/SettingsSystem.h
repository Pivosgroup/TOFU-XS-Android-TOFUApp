#pragma once
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

#include "JNIBase.h"
#include "URI.h"

class CJNIContentResolver;
class CJNISettingsSystem : public CJNIBase
{
public:
  CJNISettingsSystem(jni::jhobject const& object) : CJNIBase(object) {};
  ~CJNISettingsSystem() {};

  static std::string getString(const CJNIContentResolver&, const std::string &name);
  static bool putString(const CJNIContentResolver&, const std::string &name, const std::string &value);
  static CJNIURI getUriFor(const std::string &name);
  static int getInt(const CJNIContentResolver&, const std::string &name, int def);
  static int getInt(const CJNIContentResolver&, const std::string &name);
  static bool putInt(const CJNIContentResolver&, const std::string &name, int def);
  static int64_t getLong(const CJNIContentResolver&, const std::string &name, int64_t def);
  static int64_t getLong(const CJNIContentResolver&, const std::string &name);
  static bool putLong(const CJNIContentResolver&, const std::string &name, int64_t def);
  static float getFloat(const CJNIContentResolver&, const std::string &name, float def);
  static float getFloat(const CJNIContentResolver&, const std::string &name);
  static bool putFloat(const CJNIContentResolver&, const std::string &name, float def);
//  static void getConfiguration(const CJNIContentResolver&, android.content.res.Configuration);
//  static bool putConfiguration(const CJNIContentResolver&, android.content.res.Configuration);
  static bool getShowGTalkServiceStatus(const CJNIContentResolver&);
  static void setShowGTalkServiceStatus(const CJNIContentResolver&, bool flags);

private:
  static const char *m_classname;
};
