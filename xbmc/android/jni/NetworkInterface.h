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

class CJNIURI;

class CJNINetworkInterface : public CJNIBase
{
public:
  CJNINetworkInterface(const jni::jhobject &object) : CJNIBase(object){};
  ~CJNINetworkInterface(){};

 // std::string getName();
//  CJNIEnumeration<CJNIInetAddress>getInetAddresses();
//  CJNIList<InterfaceAddress> getInterfaceAddresses();
//  java.util.Enumeration getSubInterfaces();
//  CJNINetworkInterface getParent();
//  int getIndex();
//  std::string getDisplayName();
  static CJNINetworkInterface getByName(const std::string &name);
//  static native CJNINetworkInterface getByIndex(int);
//  static CJNINetworkInterface getByInetAddress(java.net.InetAddress)
//  static java.util.Enumeration getNetworkInterfaces()
//  bool isUp();
//  bool isLoopback();
//  bool isPointToPoint();
//  bool supportsMulticast();
  std::vector<char> getHardwareAddress();
//  int getMTU()
//  bool isVirtual();
//  bool equals(java.lang.Object);
//  int hashCode();
//  std::string toString();
private:
  static const char* m_classname;
};
