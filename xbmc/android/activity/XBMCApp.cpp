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

#include <sstream>

#include <unistd.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <string.h>

#include <android/native_window.h>
#include <android/configuration.h>
#include <jni.h>

#include "XBMCApp.h"

#include "input/MouseStat.h"
#include "input/XBMC_keysym.h"
#include "input/XBMC_vkeys.h"
#include "input/KeyboardStat.h"
#include "guilib/Key.h"
#include "windowing/XBMC_events.h"
#include <android/log.h>

#include "Application.h"
#include "settings/AdvancedSettings.h"
#include "xbmc.h"
#include "windowing/WinEvents.h"
#include "filesystem/File.h"
#include "guilib/GUIWindowManager.h"
#include "utils/log.h"
#include "ApplicationMessenger.h"
#include "utils/StringUtils.h"
#include "AppParamParser.h"
#include "XbmcContext.h"
#include <android/bitmap.h>
#include "AndroidFeatures.h"
#include "android/jni/JNIThreading.h"
#include "android/jni/BroadcastReceiver.h"
#include "android/jni/Intent.h"
#include "android/jni/PackageManager.h"
#include "android/jni/Context.h"
#include "android/jni/AudioManager.h"
#include "android/jni/PowerManager.h"
#include "android/jni/WakeLock.h"
#include "android/jni/Environment.h"
#include "android/jni/File.h"
#include "android/jni/IntentFilter.h"
#include "android/jni/NetworkInfo.h"
#include "android/jni/ConnectivityManager.h"
#include "android/jni/System.h"
#include "android/jni/ApplicationInfo.h"
#include "android/jni/StatFs.h"
#include "android/jni/BitmapDrawable.h"
#include "android/jni/Bitmap.h"
#include "android/jni/CharSequence.h"
#include "android/jni/URI.h"
#include "android/jni/Cursor.h"
#include "android/jni/ContentResolver.h"
#include "android/jni/MediaStore.h"
#include "android/jni/Window.h"
#include "android/jni/View.h"

#define GIGABYTES       1073741824

using namespace std;

template<class T, void(T::*fn)()>
void* thread_run(void* obj)
{
  (static_cast<T*>(obj)->*fn)();
  return NULL;
}
CEvent CXBMCApp::m_windowCreated;
bool CXBMCApp::m_runAsLauncher = false;
ANativeActivity *CXBMCApp::m_activity = NULL;
ANativeWindow* CXBMCApp::m_window = NULL;
int CXBMCApp::m_batteryLevel = 0;
int CXBMCApp::m_savedVolume = -1;
int CXBMCApp::m_initialVolume = 0;
bool CXBMCApp::m_moveTaskToBackWhenDone = false;

CXBMCApp::CXBMCApp(ANativeActivity* nativeActivity)
  : CJNIContext(nativeActivity)
  , CJNIBroadcastReceiver("com/pivos/tofu/XBMCBroadcastReceiver")
  , m_wakeLock(NULL)
{
  m_activity = nativeActivity;
  m_firstrun = true;
  m_exiting=false;
  m_inFront  = true;
  if (m_activity == NULL)
  {
    android_printf("CXBMCApp: invalid ANativeActivity instance");
    exit(1);
    return;
  }
}

CXBMCApp::~CXBMCApp()
{
}

void CXBMCApp::onStart()
{
  android_printf("%s: ", __PRETTY_FUNCTION__);

  // must hide before the native window's view is created.
  // do this regardless of if already started, if we have
  // been moved to the background and are shown again,
  // we get an onStart call.
  ShowStatusBar(false);

  if (!m_firstrun)
  {
    android_printf("%s: Already running, ignoring request to start", __PRETTY_FUNCTION__);
    return;
  }

  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  pthread_create(&m_thread, &attr, thread_run<CXBMCApp, &CXBMCApp::run>, this);
  pthread_attr_destroy(&attr);
}

void CXBMCApp::onResume()
{
  android_printf("%s: ", __PRETTY_FUNCTION__);

  // when resuming from being backgrounded, CXBMCApp::onStart
  // is now too early and we have not been given our window
  // back, so do the ShowStatusBar call again to take affect.
  ShowStatusBar(false);

  CJNIIntentFilter intentFilter;
  intentFilter.addAction("android.intent.action.BATTERY_CHANGED");
  intentFilter.addAction("android.intent.action.MEDIA_MOUNTED");
  intentFilter.addAction("android.net.conn.CONNECTIVITY_CHANGE");
  intentFilter.addAction("android.net.ethernet.ETH_STATE_CHANGED");
  intentFilter.addAction("android.net.wifi.STATE_CHANGE");
  intentFilter.addAction("android.net.wifi.RSSI_CHANGED");
  intentFilter.addAction("android.net.wifi.SCAN_RESULTS");
  intentFilter.addAction("android.net.wifi.NETWORK_IDS_CHANGED");
  intentFilter.addAction("android.net.wifi.supplicant.STATE_CHANGE");
  registerReceiver(*this, intentFilter);
  
  if (m_savedVolume != -1)
    SetSystemVolume(m_savedVolume);

  if (!m_firstrun && m_inFront)
  {
    XBMC_keysym sym;
    // Emulate browser_home button
    sym.scancode  = 0xb4;
    sym.sym       = XBMCK_BROWSER_HOME;
    sym.unicode   = 0x0000;
    sym.mod       = XBMCKMOD_NUM;
    CKey key(g_Keyboard.ProcessKeyDown(sym));
    g_application.OnKey(key);
  }
  
  m_inFront = true;
}

void CXBMCApp::onPause()
{
  android_printf("%s: ", __PRETTY_FUNCTION__);

  m_savedVolume = GetSystemVolume();
  // Restore android volume
  SetSystemVolume(m_initialVolume);

  unregisterReceiver(*this);
}

void CXBMCApp::onStop()
{
  android_printf("%s: ", __PRETTY_FUNCTION__);
  m_inFront = false;
}

void CXBMCApp::onDestroy()
{
  android_printf("%s", __PRETTY_FUNCTION__);

  // If android is forcing us to stop, ask XBMC to exit then wait until it's
  // been destroyed.
  if (!m_exiting)
  {
    XBMC_Stop();
    pthread_join(m_thread, NULL);
    android_printf(" => XBMC finished");
  }

  if (m_wakeLock != NULL && m_activity != NULL)
  {
    delete m_wakeLock;
    m_wakeLock = NULL;
  }
}

void CXBMCApp::onSaveState(void **data, size_t *size)
{
  android_printf("%s: ", __PRETTY_FUNCTION__);
  // no need to save anything as XBMC is running in its own thread
}

void CXBMCApp::onConfigurationChanged()
{
  android_printf("%s: ", __PRETTY_FUNCTION__);
  // ignore any configuration changes like screen rotation etc
}

void CXBMCApp::onLowMemory()
{
  android_printf("%s: ", __PRETTY_FUNCTION__);
  // can't do much as we don't want to close completely
}

void CXBMCApp::onCreateWindow(ANativeWindow* window)
{
  android_printf("%s: ", __PRETTY_FUNCTION__);
  if (window == NULL)
  {
    android_printf(" => invalid ANativeWindow object");
    return;
  }
  m_window = window;
  m_windowCreated.Set();
  if (getWakeLock() &&  m_wakeLock)
    m_wakeLock->acquire();
  if(!m_firstrun)
  {
    XBMC_SetupDisplay();
    XBMC_Pause(false);
  }
}

void CXBMCApp::onResizeWindow()
{
  android_printf("%s: ", __PRETTY_FUNCTION__);
  m_window=NULL;
  m_windowCreated.Reset();
  // no need to do anything because we are fixed in fullscreen landscape mode
}

void CXBMCApp::onDestroyWindow()
{
  android_printf("%s: ", __PRETTY_FUNCTION__);
  m_window=NULL;
  m_windowCreated.Reset();
  // If we have exited XBMC, it no longer exists.
  if (!m_exiting)
  {
    XBMC_DestroyDisplay();
    XBMC_Pause(true);
  }

  if (m_wakeLock)
    m_wakeLock->release();

}

void CXBMCApp::onGainFocus()
{
  android_printf("%s: ", __PRETTY_FUNCTION__);
}

void CXBMCApp::onLostFocus()
{
  android_printf("%s: ", __PRETTY_FUNCTION__);
}

bool CXBMCApp::getWakeLock()
{
  if (m_wakeLock)
    return true;

  m_wakeLock = new CJNIWakeLock(CJNIPowerManager(getSystemService("power")).newWakeLock("com.pivos.tofu"));

  return true;
}

void CXBMCApp::run()
{
  int status = 0;

  SetupEnv();
  XBMC::Context context;
  GetSUPath();

  m_initialVolume = GetSystemVolume();

  CJNIIntent startIntent = getIntent();
  android_printf("XBMC Started with action: %s\n",startIntent.getAction().c_str());

  std::string filenameToPlay = GetFilenameFromIntent(startIntent);
  if (!filenameToPlay.empty())
  {
    int argc = 2;
    const char** argv = (const char**) malloc(argc*sizeof(char*));

    std::string exe_name("XBMC");
    argv[0] = exe_name.c_str();
    argv[1] = filenameToPlay.c_str();

    CAppParamParser appParamParser;
    appParamParser.Parse((const char **)argv, argc);

    free(argv);
  }

  if (startIntent.hasCategory("android.intent.category.HOME"))
  {
    m_runAsLauncher = true;
    WaitForMedia(30000);
  }

  m_firstrun=false;
  android_printf(" => running XBMC_Run...");
  try
  {
    status = XBMC_Run(true);
    android_printf(" => XBMC_Run finished with %d", status);
  }
  catch(...)
  {
    android_printf("ERROR: Exception caught on main loop. Exiting");
  }

  // If we are have not been force by Android to exit, notify its finish routine.
  // This will cause android to run through its teardown events, it calls:
  // onPause(), onLostFocus(), onDestroyWindow(), onStop(), onDestroy().
  ANativeActivity_finish(m_activity);
  m_exiting=true;
}

void CXBMCApp::XBMC_Pause(bool pause)
{
  android_printf("XBMC_Pause(%s)", pause ? "true" : "false");
  // Only send the PAUSE action if we are pausing XBMC and something is currently playing
  if (pause && g_application.m_pPlayer->IsPlaying() && !g_application.m_pPlayer->IsPaused())
    CApplicationMessenger::Get().SendAction(CAction(ACTION_PAUSE), WINDOW_INVALID, true);
  else
  {
    if (!pause && g_application.m_pPlayer->IsPaused())
      CApplicationMessenger::Get().SendAction(CAction(ACTION_PLAYER_PLAYPAUSE), WINDOW_INVALID, true);
  }
}

void CXBMCApp::XBMC_Stop()
{
  CApplicationMessenger::Get().Quit();
}

bool CXBMCApp::XBMC_SetupDisplay()
{
  android_printf("XBMC_SetupDisplay()");
  return CApplicationMessenger::Get().SetupDisplay();
}

bool CXBMCApp::XBMC_DestroyDisplay()
{
  android_printf("XBMC_DestroyDisplay()");
  return CApplicationMessenger::Get().DestroyDisplay();
}

int CXBMCApp::SetBuffersGeometry(int width, int height, int format)
{
  return ANativeWindow_setBuffersGeometry(m_window, width, height, format);
}

int CXBMCApp::android_printf(const char *format, ...)
{
  // For use before CLog is setup by XBMC_Run()
  va_list args;
  va_start(args, format);
  int result = __android_log_vprint(ANDROID_LOG_VERBOSE, "XBMC", format, args);
  va_end(args);
  return result;
}

int CXBMCApp::GetDPI()
{
  if (m_activity == NULL || m_activity->assetManager == NULL)
    return 0;

  // grab DPI from the current configuration - this is approximate
  // but should be close enough for what we need
  AConfiguration *config = AConfiguration_new();
  AConfiguration_fromAssetManager(config, m_activity->assetManager);
  int dpi = AConfiguration_getDensity(config);
  AConfiguration_delete(config);

  return dpi;
}

const std::string CXBMCApp::GetSUPath()
{
  static int m_isRooted = -1;
  static std::string su_path;
  if (m_isRooted == -1)
  {
    // check the standard location
    su_path = "/system/bin/su";
    m_isRooted = XFILE::CFile::Exists(su_path) ? 1 : 0;

    // if not found check the alternate location
    if (!m_isRooted)
    {
      su_path = "/system/xbin/su";
      m_isRooted = XFILE::CFile::Exists(su_path) ? 1 : 0;
    }

    // if not found, we are not rooted
    if (!m_isRooted)
      su_path = "";
    else
      android_printf("GetSUPath: found su at %s", su_path.c_str());
  }

  return su_path;
}

void CXBMCApp::ShowStatusBar(bool show)
{
  int version = CAndroidFeatures::GetVersion();
  int flags = 0;

  android_printf("%s: show(%i)", __PRETTY_FUNCTION__, show);
  CJNIWindow window = getWindow();
  if (!window)
    return;

  CJNIView view = window.getDecorView();
  if(!view)
    return;

  if (show)
    flags = CJNIView::SYSTEM_UI_FLAG_VISIBLE;
  else
  {
    flags = CJNIView::SYSTEM_UI_FLAG_HIDE_NAVIGATION;
    flags |= CJNIView::SYSTEM_UI_FLAG_LOW_PROFILE;
    if (version >= 16)
    {
      flags |= CJNIView::SYSTEM_UI_FLAG_FULLSCREEN;
      flags |= CJNIView::SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN;
      flags |= CJNIView::SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION;
    }
  }
  if (view.getSystemUiVisibility() != flags)
  {
    android_printf("%s: setSystemUiVisibility(%i)", __PRETTY_FUNCTION__, flags);
    view.setSystemUiVisibility(flags);
  }
}

void CXBMCApp::PlayBackEnded()
{
  if (m_runAsLauncher && m_moveTaskToBackWhenDone)
  {
    android_printf("%s: moveTaskToBack", __PRETTY_FUNCTION__);
    m_moveTaskToBackWhenDone = false;
    moveTaskToBack(true);
  }
}

void CXBMCApp::onSystemUiVisibilityChange(int visibility)
{
  android_printf("%s: visibility(%i)", __PRETTY_FUNCTION__, visibility);
  if (visibility == CJNIView::SYSTEM_UI_FLAG_VISIBLE)
    ShowStatusBar(false);
}

bool CXBMCApp::ListApplications(vector<androidPackage> *applications)
{
  CJNIList<CJNIApplicationInfo> packageList = GetPackageManager().getInstalledApplications(CJNIPackageManager::GET_ACTIVITIES);
  if (!packageList)
    return false;

  int numPackages = packageList.size();
  for (int i = 0; i < numPackages; i++)
  {
    androidPackage newPackage;
    newPackage.packageName = packageList.get(i).packageName;
    newPackage.packageLabel = GetPackageManager().getApplicationLabel(packageList.get(i)).toString();
    CJNIIntent intent = GetPackageManager().getLaunchIntentForPackage(newPackage.packageName);
    if (!intent || !intent.hasCategory("android.intent.category.LAUNCHER"))
      continue;

    applications->push_back(newPackage);
  }

  return true;
}

bool CXBMCApp::GetIconSize(const string &packageName, int *width, int *height)
{
  JNIEnv* env = xbmc_jnienv();
  AndroidBitmapInfo info;
  CJNIBitmapDrawable drawable = (CJNIBitmapDrawable)GetPackageManager().getApplicationIcon(packageName);
  CJNIBitmap icon(drawable.getBitmap());
  AndroidBitmap_getInfo(env, icon.get_raw(), &info);
  *width = info.width;
  *height = info.height;
  return true;
}

bool CXBMCApp::GetIcon(const string &packageName, void* buffer, unsigned int bufSize)
{
  void *bitmapBuf = NULL;
  JNIEnv* env = xbmc_jnienv();
  CJNIBitmapDrawable drawable = (CJNIBitmapDrawable)GetPackageManager().getApplicationIcon(packageName);
  CJNIBitmap bitmap(drawable.getBitmap());
  AndroidBitmap_lockPixels(env, bitmap.get_raw(), &bitmapBuf);
  if (bitmapBuf)
  {
    memcpy(buffer, bitmapBuf, bufSize);
    AndroidBitmap_unlockPixels(env, bitmap.get_raw());
    return true;
  }
  return false;
}

bool CXBMCApp::HasLaunchIntent(const string &package)
{
  return GetPackageManager().getLaunchIntentForPackage(package) != NULL;
}

// Note intent, dataType, dataURI all default to ""
bool CXBMCApp::StartActivity(const string &package, const string &intent, const string &dataType, const string &dataURI)
{
  CJNIIntent newIntent = intent.empty() ?
    GetPackageManager().getLaunchIntentForPackage(package) : CJNIIntent(intent);

  if (!newIntent)
    return false;

  if (!dataURI.empty())
  {
    CJNIURI jniURI = CJNIURI::parse(dataURI);

    if (!jniURI)
      return false;

    newIntent.setDataAndType(jniURI, dataType); 
  }

  newIntent.setPackage(package);
  startActivity(newIntent);
  if (xbmc_jnienv()->ExceptionOccurred())
  {
    CLog::Log(LOGERROR, "CXBMCApp::StartActivity - ExceptionOccurred launching %s", package.c_str());
    xbmc_jnienv()->ExceptionClear();
    return false;
  }

  return true;
}

bool CXBMCApp::StartActivityByComponent(const string &package, const string &component)
{
  CJNIIntent newIntent = CJNIIntent();
  if (!newIntent)
    return false;

  CJNIComponentName componentName(package, component);
  newIntent.setComponent(componentName);
  startActivity(newIntent);
  if (xbmc_jnienv()->ExceptionOccurred())
  {
    CLog::Log(LOGERROR, "CXBMCApp::StartActivityByComponent - ExceptionOccurred launching %s:%s",
      package.c_str(), component.c_str());
    xbmc_jnienv()->ExceptionClear();
    return false;
  }

  return true;
}

int CXBMCApp::GetBatteryLevel()
{
  return m_batteryLevel;
}

bool CXBMCApp::GetExternalStorage(std::string &path, const std::string &type /* = "" */)
{
  std::string sType;
  std::string mountedState;
  bool mounted = false;

  if(type == "files" || type.empty())
  {
    CJNIFile external = CJNIEnvironment::getExternalStorageDirectory();
    if (external)
      path = external.getAbsolutePath();
  }
  else
  {
    if (type == "music")
      sType = "Music"; // Environment.DIRECTORY_MUSIC
    else if (type == "videos")
      sType = "Movies"; // Environment.DIRECTORY_MOVIES
    else if (type == "pictures")
      sType = "Pictures"; // Environment.DIRECTORY_PICTURES
    else if (type == "photos")
      sType = "DCIM"; // Environment.DIRECTORY_DCIM
    else if (type == "downloads")
      sType = "Download"; // Environment.DIRECTORY_DOWNLOADS
    if (!sType.empty())
    {
      CJNIFile external = CJNIEnvironment::getExternalStoragePublicDirectory(sType);
      if (external)
        path = external.getAbsolutePath();
    }
  }
  mountedState = CJNIEnvironment::getExternalStorageState();
  mounted = (mountedState == "mounted" || mountedState == "mounted_ro");
  return mounted && !path.empty();
}

bool CXBMCApp::GetStorageUsage(const std::string &path, std::string &usage)
{
  if (path.empty())
  {
    std::ostringstream fmt;
    fmt.width(24);  fmt << std::left  << "Filesystem";
    fmt.width(12);  fmt << std::right << "Size";
    fmt.width(12);  fmt << "Used";
    fmt.width(12);  fmt << "Avail";
    fmt.width(12);  fmt << "Use %";

    usage = fmt.str();
    return false;
  }

  CJNIStatFs fileStat(path);
  int blockSize = fileStat.getBlockSize();
  int blockCount = fileStat.getBlockCount();
  int freeBlocks = fileStat.getFreeBlocks();

  if (blockSize <= 0 || blockCount <= 0 || freeBlocks < 0)
    return false;

  float totalSize = (float)blockSize * blockCount / GIGABYTES;
  float freeSize = (float)blockSize * freeBlocks / GIGABYTES;
  float usedSize = totalSize - freeSize;
  float usedPercentage = usedSize / totalSize * 100;

  std::ostringstream fmt;
  fmt << std::fixed;
  fmt.precision(1);
  fmt.width(24);  fmt << std::left  << path;
  fmt.width(12);  fmt << std::right << totalSize << "G"; // size in GB
  fmt.width(12);  fmt << usedSize << "G"; // used in GB
  fmt.width(12);  fmt << freeSize << "G"; // free
  fmt.precision(0);
  fmt.width(12);  fmt << usedPercentage << "%"; // percentage used

  usage = fmt.str();
  return true;
}

// Used in Application.cpp to figure out volume steps
int CXBMCApp::GetMaxSystemVolume()
{
  JNIEnv* env = xbmc_jnienv();
  static int maxVolume = -1;
  if (maxVolume == -1)
  {
    maxVolume = GetMaxSystemVolume(env);
  }
  //android_printf("CXBMCApp::GetMaxSystemVolume: %i",maxVolume);
  return maxVolume;
}

int CXBMCApp::GetMaxSystemVolume(JNIEnv *env)
{
  CJNIAudioManager audioManager(getSystemService("audio"));
  if (audioManager)
    return audioManager.getStreamMaxVolume();
  android_printf("CXBMCApp::SetSystemVolume: Could not get Audio Manager");
  return 0;
}

int CXBMCApp::GetSystemVolume()
{
  CJNIAudioManager audioManager(getSystemService("audio"));
  if (audioManager)
    return audioManager.getStreamVolume();
  else 
  {
    android_printf("CXBMCApp::GetSystemVolume: Could not get Audio Manager");
    return 0;
  }
}

void CXBMCApp::SetSystemVolume(int val)
{
  CJNIAudioManager audioManager(getSystemService("audio"));
  if (audioManager)
    audioManager.setStreamVolume(val);
  else
    android_printf("CXBMCApp::SetSystemVolume: Could not get Audio Manager");
}

void CXBMCApp::SetSystemVolume(JNIEnv *env, float percent)
{
  CJNIAudioManager audioManager(getSystemService("audio"));
  int maxVolume = (int)(GetMaxSystemVolume() * percent);
  if (audioManager)
    audioManager.setStreamVolume(maxVolume);
  else
    android_printf("CXBMCApp::SetSystemVolume: Could not get Audio Manager");
}

void CXBMCApp::onReceive(CJNIIntent intent)
{
  std::string action = intent.getAction();
  android_printf("CXBMCApp::onReceive Got intent. Action: %s", action.c_str());
  if (action == "android.intent.action.BATTERY_CHANGED")
    m_batteryLevel = intent.getIntExtra("level",-1);
  else if (action == "android.intent.action.MEDIA_MOUNTED")
    m_mediaMounted.Set();
  else if (action == "android.net.wifi.STATE_CHANGE"  ||
           action == "android.net.conn.CONNECTIVITY_CHANGE" ||
           action == "android.net.ethernet.ETH_STATE_CHANGED" ||
           action == "android.net.wifi.RSSI_CHANGED"        ||
           action == "android.net.wifi.SCAN_RESULTS"        ||
           action == "android.net.wifi.supplicant.STATE_CHANGE" ||
           action == "android.net.wifi.NETWORK_IDS_CHANGED")
  {
    const NetworkEventDataPtr intentPtr = NetworkEventDataPtr(new CJNIIntent(intent));
    g_application.getNetwork().ReceiveNetworkEvent(intentPtr);
  }
}

void CXBMCApp::onNewIntent(CJNIIntent intent)
{
  std::string action = intent.getAction();
  if (action == "android.intent.action.VIEW")
  {
    std::string playFile = GetFilenameFromIntent(intent);
    if (!playFile.empty())
    {
      android_printf("CXBMCApp::onNewIntent: filename(%s)", playFile.c_str());
      CApplicationMessenger::Get().MediaPlay(playFile);
      // if we are a launcher, push us into the back of the activity stack when done.
      // this acts like a 'finish' but does not cause us to exit.
      if (m_runAsLauncher)
        m_moveTaskToBackWhenDone = true;
    }
    else if (m_runAsLauncher)
    {
      // if we are a launcher, and someone told us to play something but
      // we could not resolve it, just return back to them.
      moveTaskToBack(true);
    }
  }
}

void CXBMCApp::SetupEnv()
{
  setenv("XBMC_ANDROID_SYSTEM_LIBS", CJNISystem::getProperty("java.library.path").c_str(), 0);
  setenv("XBMC_ANDROID_DATA", getApplicationInfo().dataDir.c_str(), 0);
  setenv("XBMC_ANDROID_LIBS", getApplicationInfo().nativeLibraryDir.c_str(), 0);
  setenv("XBMC_ANDROID_APK", getPackageResourcePath().c_str(), 0);

  std::string cacheDir = getCacheDir().getAbsolutePath();
  setenv("XBMC_BIN_HOME", (cacheDir + "/apk/assets").c_str(), 0);
  setenv("XBMC_HOME", (cacheDir + "/apk/assets").c_str(), 0);

  std::string externalDir;
  CJNIFile androidPath = getExternalFilesDir("");
  if (!androidPath)
    androidPath = getDir("com.pivos.tofu", 1);

  if (androidPath)
    externalDir = androidPath.getAbsolutePath();

  if (!externalDir.empty())
    setenv("HOME", externalDir.c_str(), 0);
  else
    setenv("HOME", getenv("XBMC_TEMP"), 0);
}

std::string CXBMCApp::GetFilenameFromIntent(const CJNIIntent &intent)
{
    std::string ret;
    if (!intent)
      return ret;
    CJNIURI data = intent.getData();
    if (!data)
      return ret;
    std::string scheme = data.getScheme();
    StringUtils::ToLower(scheme);
    if (scheme == "content")
    {
      std::vector<std::string> filePathColumn;
      filePathColumn.push_back(CJNIMediaStoreMediaColumns::DATA);
      CJNICursor cursor = getContentResolver().query(data, filePathColumn, std::string(), std::vector<std::string>(), std::string());
      if(cursor.moveToFirst())
      {
        int columnIndex = cursor.getColumnIndex(filePathColumn[0]);
        ret = cursor.getString(columnIndex);
      }
      cursor.close();
    }
    else if(scheme == "file")
      ret = data.getPath();
    else
      ret = data.toString();
  return ret;
}

bool CXBMCApp::WaitForMedia(int timeout)
{
  android_printf("CXBMCApp::WaitForMedia Waiting for storage");
  if (CJNIEnvironment::getExternalStorageState() == "mounted")
    return true;
  m_mediaMounted.WaitMSec(timeout);
  return CJNIEnvironment::getExternalStorageState() == "mounted";
 }

const ANativeWindow** CXBMCApp::GetNativeWindow(int timeout)
{
  if (m_window)
    return (const ANativeWindow**)&m_window;

  m_windowCreated.WaitMSec(timeout);
  return (const ANativeWindow**)&m_window;
}
