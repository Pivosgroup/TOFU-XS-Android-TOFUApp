#pragma once
/*
 *      Copyright (C) 2012-2013 Team XBMC
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

#include <math.h>
#include <pthread.h>
#include <string>
#include <vector>

#include <android/native_activity.h>

#include "IActivityHandler.h"
#include "IInputHandler.h"

#include "xbmc.h"
#include "android/jni/Context.h"
#include "android/jni/BroadcastReceiver.h"
#include "threads/Event.h"

// forward delares
class CJNIWakeLock;
class CAESinkAUDIOTRACK;
typedef struct _JNIEnv JNIEnv;

struct androidIcon
{
  unsigned int width;
  unsigned int height;
  void *pixels;
};  

struct androidPackage
{
  std::string packageName;
  std::string packageLabel;
};


class CXBMCApp : public IActivityHandler, public CJNIContext, public CJNIBroadcastReceiver
{
public:
  CXBMCApp(ANativeActivity *nativeActivity);
  virtual ~CXBMCApp();
  virtual void onReceive(CJNIIntent intent);
  virtual void onNewIntent(CJNIIntent intent);
  virtual void onSystemUiVisibilityChange(int visibility);

  bool isValid() { return m_activity != NULL; }

  void onStart();
  void onResume();
  void onPause();
  void onStop();
  void onDestroy();

  void onSaveState(void **data, size_t *size);
  void onConfigurationChanged();
  void onLowMemory();

  void onCreateWindow(ANativeWindow* window);
  void onResizeWindow();
  void onDestroyWindow();
  void onGainFocus();
  void onLostFocus();


  static const ANativeWindow** GetNativeWindow(int timeout);
  static int SetBuffersGeometry(int width, int height, int format);
  static int android_printf(const char *format, ...);
  
  static int GetBatteryLevel();
  static bool StartActivity(const std::string &package, const std::string &intent = std::string(), const std::string &dataType = std::string(), const std::string &dataURI = std::string());
  static bool StartActivityByComponent(const std::string &package, const std::string &component);
  static bool ListApplications(std::vector <androidPackage> *applications);
  static bool GetIconSize(const std::string &packageName, int *width, int *height);
  static bool GetIcon(const std::string &packageName, void* buffer, unsigned int bufSize); 

  /*!
   * \brief If external storage is available, it returns the path for the external storage (for the specified type)
   * \param path will contain the path of the external storage (for the specified type)
   * \param type optional type. Possible values are "", "files", "music", "videos", "pictures", "photos, "downloads"
   * \return true if external storage is available and a valid path has been stored in the path parameter
   */
  static bool GetExternalStorage(std::string &path, const std::string &type = "");
  static bool GetStorageUsage(const std::string &path, std::string &usage);
  static int GetMaxSystemVolume();
  static int GetSystemVolume();
  static void SetSystemVolume(int val);

  static int GetDPI();
  static const std::string GetSUPath();

  static bool WaitForNativeWindow(int timeout);
  bool WaitForMedia(int timeout);
  static bool IsLauncher() { return m_runAsLauncher;};
  static void ShowStatusBar(bool show);

protected:
  // limit who can access Volume
  friend class CAESinkAUDIOTRACK;

  static int GetMaxSystemVolume(JNIEnv *env);
  static void SetSystemVolume(JNIEnv *env, float percent);

private:
  static bool HasLaunchIntent(const std::string &package);
  bool getWakeLock();
  std::string GetFilenameFromIntent(const CJNIIntent &intent);
  void run();
  void stop();
  void SetupEnv();
  static ANativeActivity *m_activity;
  CJNIWakeLock *m_wakeLock;
  static int m_batteryLevel;  
  static int m_initialVolume;  
  bool m_firstrun;
  bool m_exiting;
  pthread_t m_thread;
  CEvent m_mediaMounted;
  static ANativeWindow* m_window;
  static CEvent m_windowCreated;
  static bool m_runAsLauncher;
  
  void XBMC_Pause(bool pause);
  void XBMC_Stop();
  bool XBMC_DestroyDisplay();
  bool XBMC_SetupDisplay();
};
