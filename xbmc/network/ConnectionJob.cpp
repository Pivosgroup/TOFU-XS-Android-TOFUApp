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

#include "ConnectionJob.h"
#include "Application.h"
#include "utils/log.h"
#include "dialogs/GUIDialogOK.h"
#include "dialogs/GUIDialogBusy.h"
#include "guilib/GUIWindowManager.h"
#include "guilib/LocalizeStrings.h"

CConnectionJob::CConnectionJob(CConnectionPtr connection, const CIPConfig &ipconfig)
{
  m_ipconfig = ipconfig;
  m_connection = connection;
}

bool CConnectionJob::DoWork()
{
  bool result;

  CGUIDialogBusy *busy_dialog = (CGUIDialogBusy*)g_windowManager.GetWindow(WINDOW_DIALOG_BUSY);
  busy_dialog->Show();

  // we need to shutdown network services before changing the connection.
  // The Network Manager's PumpNetworkEvents will take care of starting them back up.
  g_application.getNetwork().OnConnectionStateChange(NETWORK_CONNECTION_STATE_DISCONNECTED);
  result = m_connection->Connect(g_application.getNetwork().GetPassphraseStorage(), m_ipconfig);
  if (!result)
  {
    // the connect failed, pop a failed dialog
    // and revert to the previous connection.
    // <string id="13297">Not connected. Check network settings.</string>
    CGUIDialogOK::ShowAndGetInput(0, 13297, 0, 0);
  }

  busy_dialog->Close();
  return result;
}
