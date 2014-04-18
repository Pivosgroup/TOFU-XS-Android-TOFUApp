#pragma once
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


#include <string>
class IConnection;
class IPassphraseStorage
{
public:
  virtual ~IPassphraseStorage() { }

  /*!
   \brief HasStoredPassphrase

   It may be useful to know if there is already a stored passphrase.
   For example, A wifimanager may wish to have a "forget" button, that is only
   visible if a passphrase has already been stored.
  */
  virtual bool HasStoredPassphrase(const std::string &uuid, IConnection *connection = NULL) = 0;

  /*!
   \brief Invalidate passphrase
   
    While a connection is connecting it may invalidate any stored passphrase. This is used
    if the subsystem is not able to store passwords but the connection may have changed passphrase

   \param uuid the unique id of the connection associated with the passphrase
   \sa IConnection
   */
  virtual void InvalidatePassphrase(const std::string &uuid, IConnection *connection = NULL) = 0;

  /*!
   \brief Get passphrase
   
    While a connection is connecting it may need to acquire a passphrase. This is used
    if the subsystem has no stored passphrase.

   \param uuid the unique id of the connection.
   \param passphrase a string which the passphrase storage will fill in the passphrase.
   \return true if the passphrase was filled and false if it was some form of failure acquiring it.
   \sa IConnection
   */
  virtual bool GetPassphrase(const std::string &uuid, std::string &passphrase, IConnection *connection = NULL) = 0;

  /*!
   \brief Store passphrase
   
    While a connection is connecting it may need to store a passphrase. This is used
    if the subsystem is not capable of storing the passphrase itself.

   \param uuid the unique id of the connection.
   \param passphrase is the passphrase to be stored
   \sa IConnection
   */
  virtual void StorePassphrase(const std::string &uuid, const std::string &passphrase, IConnection *connection = NULL) = 0;
};
