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

#include "NetworkInterface.h"
#include "jutils/jutils-details.hpp"

using namespace jni;
const char* CJNINetworkInterface::m_classname("java/net/NetworkInterface");

CJNINetworkInterface CJNINetworkInterface::getByName(const std::string &name)
{
  return call_static_method<jhobject>(m_classname,
    "getByName", "(Ljava/lang/String;)Ljava/net/NetworkInterface;",
    jcast<jhstring>(name));
}

std::vector<char> CJNINetworkInterface::getHardwareAddress()
{
  jhbyteArray array = call_method<jhbyteArray>(m_object,
    "getHardwareAddress", "()[B");

  JNIEnv *env = xbmc_jnienv();
  jsize size = env->GetArrayLength(array.get());
  std::vector<char> chararray;
  chararray.resize(size);
  env->GetByteArrayRegion(array.get(), 0, size, (jbyte*)chararray.data());
  return chararray;
}


