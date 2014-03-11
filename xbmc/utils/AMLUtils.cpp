/*
 *      Copyright (C) 2011-2013 Team XBMC
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

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string>

#include "rendering/RenderSystem.h"
#include "settings/MediaSettings.h"
#include "utils/AMLUtils.h"
#include "utils/CPUInfo.h"
#include "utils/log.h"
#include "utils/StringUtils.h"
#include "guilib/StereoscopicsManager.h"

#define MODE_3D_DISABLE         0x00000000
#define MODE_3D_LR              0x00000101
#define MODE_3D_LR_SWITCH       0x00000501
#define MODE_3D_BT              0x00000201
#define MODE_3D_BT_SWITCH       0x00000601
#define MODE_3D_TO_2D_L         0x00000102
#define MODE_3D_TO_2D_R         0x00000902
#define MODE_3D_TO_2D_T         0x00000202
#define MODE_3D_TO_2D_B         0x00000a02

static void aml_set_video_3d_mode(const int mode3d)
{
  char mode[16] = {};
  snprintf(mode, sizeof(mode), "0x%08x", mode3d);
  CLog::Log(LOGDEBUG, "aml_set_video_3d_mode: %s", mode);
  aml_set_sysfs_str("/sys/class/ppmgr/ppmgr_3d_mode", mode);
}

static void aml_hdmi_3D_mode(const char *mode3d)
{
  static bool reset_disp_mode = false;
  CLog::Log(LOGDEBUG, "aml_hdmi_3D_mode: %s", mode3d);
  aml_set_sysfs_str("/sys/class/amhdmitx/amhdmitx0/config", mode3d);
  if (strstr(mode3d, "3doff"))
  {
    if (reset_disp_mode)
    {
      // Some 3D HDTVs will not exit from 3D mode with 3doff
      char disp_mode[256] = {};
      if (aml_get_sysfs_str("/sys/class/display/mode", disp_mode, 255) != -1)
        aml_set_sysfs_str("/sys/class/amhdmitx/amhdmitx0/disp_mode", disp_mode);

      reset_disp_mode = false;
    }
  }
  else
    reset_disp_mode = true;
}

int aml_set_sysfs_str(const char *path, const char *val)
{
  int fd = open(path, O_CREAT | O_RDWR | O_TRUNC, 0644);
  if (fd >= 0)
  {
    write(fd, val, strlen(val));
    close(fd);
    return 0;
  }
  return -1;
}

int aml_get_sysfs_str(const char *path, char *valstr, const int size)
{
  int fd = open(path, O_RDONLY);
  if (fd >= 0)
  {
    read(fd, valstr, size - 1);
    valstr[strlen(valstr)] = '\0';
    close(fd);
    return 0;
  }

  sprintf(valstr, "%s", "fail");
  return -1;
}

int aml_set_sysfs_int(const char *path, const int val)
{
  int fd = open(path, O_CREAT | O_RDWR | O_TRUNC, 0644);
  if (fd >= 0)
  {
    char bcmd[16];
    sprintf(bcmd, "%d", val);
    write(fd, bcmd, strlen(bcmd));
    close(fd);
    return 0;
  }
  return -1;
}

int aml_get_sysfs_int(const char *path)
{
  int val = -1;
  int fd = open(path, O_RDONLY);
  if (fd >= 0)
  {
    char bcmd[16];
    read(fd, bcmd, sizeof(bcmd));
    val = strtol(bcmd, NULL, 0);
    close(fd);
  }
  return val;
}

bool aml_present()
{
  static int has_aml = -1;
  if (has_aml == -1)
  {
    int rtn = aml_get_sysfs_int("/sys/class/audiodsp/digital_raw");
    if (rtn != -1)
      has_aml = 1;
    else
      has_aml = 0;
    if (has_aml)
      CLog::Log(LOGNOTICE, "aml_present, rtn(%d)", rtn);
  }
  return has_aml == 1;
}

bool aml_hw3d_present()
{
  static int has_hw3d = -1;
  if (has_hw3d == -1)
  {
    if (aml_get_sysfs_int("/sys/class/ppmgr/ppmgr_3d_mode") != -1)
      has_hw3d = 1;
    else
      has_hw3d = 0;
  }
  return has_hw3d == 1;
}

bool aml_supports_stereo(const int mode)
{
  static int last_mode = -1;
  static bool last_rtn = false;
  if (last_mode == mode)
    return last_rtn;

  CLog::Log(LOGDEBUG, "aml_supports_stereo:mode(0x%x)", mode);
  char disp_cap_3d[256] = {};
  if (aml_get_sysfs_str("/sys/class/amhdmitx/amhdmitx0/disp_cap_3d", disp_cap_3d, 255) == -1)
  {
    last_rtn = false;
    last_mode = -1;
    return last_rtn;
  }

  if (mode == RENDER_STEREO_MODE_INTERLACED && strstr(disp_cap_3d,"FramePacking"))
    last_rtn = true;
  else if (mode == RENDER_STEREO_MODE_SPLIT_HORIZONTAL && strstr(disp_cap_3d,"TopBottom"))
    last_rtn = true;
  else if (mode == RENDER_STEREO_MODE_SPLIT_VERTICAL && strstr(disp_cap_3d,"SidebySide"))
    last_rtn = true;
  else if ( mode == RENDER_STEREO_MODE_MONO ||
            mode == RENDER_STEREO_MODE_OFF)
    last_rtn = true;
  else
    last_rtn = false;

  last_mode = mode;

  return last_rtn;
}

void aml_set_stereo_mode(const int mode, const int view)
{
  static int  last_mode   = -1;
  static bool last_invert = false;
  bool        invert      = CMediaSettings::Get().GetCurrentVideoSettings().m_StereoInvert;

  // do nothing if mode matches last time someone called us.
  if (last_mode == mode &&
      last_invert == invert)
    return;

  last_mode   = mode;
  last_invert = invert;

  CLog::Log(LOGDEBUG, "aml_set_stereo_mode:mode(0x%x)", mode);
  if (!aml_supports_stereo(mode))
    return;

  switch(mode)
  {
    case RENDER_STEREO_MODE_SPLIT_VERTICAL:
      aml_set_video_3d_mode(MODE_3D_DISABLE);
      aml_hdmi_3D_mode("3dlr");
      break;
    case RENDER_STEREO_MODE_SPLIT_HORIZONTAL:
      aml_set_video_3d_mode(MODE_3D_DISABLE);
      aml_hdmi_3D_mode("3dtb");
      break;

    case RENDER_STEREO_MODE_INTERLACED:
      {
        switch(CMediaSettings::Get().GetCurrentVideoSettings().m_StereoMode)
        {
          case RENDER_STEREO_MODE_SPLIT_VERTICAL:
            if (invert)
              aml_set_video_3d_mode(MODE_3D_LR_SWITCH);
            else
              aml_set_video_3d_mode(MODE_3D_LR);
            aml_hdmi_3D_mode("3dlr");
            break;
          case RENDER_STEREO_MODE_SPLIT_HORIZONTAL:
            if (invert)
              aml_set_video_3d_mode(MODE_3D_BT_SWITCH);
            else
              aml_set_video_3d_mode(MODE_3D_BT);
            aml_hdmi_3D_mode("3dtb");
            break;
          default:
            aml_set_video_3d_mode(MODE_3D_DISABLE);
            aml_hdmi_3D_mode("3doff");
            break;
        }
      }
      break;
    case RENDER_STEREO_MODE_MONO:
      {
        int   stream_mode = (int)CStereoscopicsManager::Get().GetStereoModeOfPlayingVideo();
        
        switch (stream_mode)
        {
          case RENDER_STEREO_MODE_SPLIT_VERTICAL:
            if (invert)
              aml_set_video_3d_mode(MODE_3D_TO_2D_R);
            else
              aml_set_video_3d_mode(MODE_3D_TO_2D_L);
            break;
          case RENDER_STEREO_MODE_SPLIT_HORIZONTAL:
            if (invert)
              aml_set_video_3d_mode(MODE_3D_TO_2D_B);
            else
              aml_set_video_3d_mode(MODE_3D_TO_2D_T);
            break;
          default:
            aml_set_video_3d_mode(MODE_3D_DISABLE);
            break;
        }
        aml_hdmi_3D_mode("3doff");
      }
      break;
    default:
      aml_set_video_3d_mode(MODE_3D_DISABLE);
      aml_hdmi_3D_mode("3doff");
      break;
  }

  return;
}

bool aml_wired_present()
{
  static int has_wired = -1;
  if (has_wired == -1)
  {
    char test[64] = {0};
    if (aml_get_sysfs_str("/sys/class/net/eth0/operstate", test, 63) != -1)
      has_wired = 1;
    else
      has_wired = 0;
  }
  return has_wired == 1;
}

void aml_permissions()
{
  if (!aml_present())
    return;

  // most all aml devices are already rooted.
  int ret = system("ls /system/xbin/su");
  if (ret != 0)
  {
    CLog::Log(LOGWARNING, "aml_permissions: missing su, playback might fail");
  }
  else
  {
    // certain aml devices have 664 permission, we need 666.
    system("su -c chmod 666 /sys/class/video/axis");
    system("su -c chmod 666 /sys/class/video/screen_mode");
    system("su -c chmod 666 /sys/class/video/disable_video");
    system("su -c chmod 666 /sys/class/tsync/pts_pcrscr");
    system("su -c chmod 666 /sys/class/audiodsp/digital_raw");
    system("su -c chmod 666 /sys/class/ppmgr/ppmgr_3d_mode");
    system("su -c chmod 666 /sys/devices/system/cpu/cpu0/cpufreq/scaling_min_freq");
    system("su -c chmod 666 /sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq");
    system("su -c chmod 666 /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor");
    CLog::Log(LOGINFO, "aml_permissions: permissions changed");
  }
}

int aml_get_cputype()
{
  static int aml_cputype = -1;
  if (aml_cputype == -1)
  {
    std::string cpu_hardware = g_cpuInfo.getCPUHardware();

    // default to AMLogic M1
    aml_cputype = 1;
    if (cpu_hardware.find("MESON-M3") != std::string::npos)
      aml_cputype = 3;
    else if (cpu_hardware.find("MESON3") != std::string::npos)
      aml_cputype = 3;
    else if (cpu_hardware.find("Meson6") != std::string::npos)
      aml_cputype = 6;
  }

  return aml_cputype;
}

void aml_cpufreq_min(bool limit)
{
// do not touch scaling_min_freq on android
#if !defined(TARGET_ANDROID)
  // only needed for m1/m3 SoCs
  if (aml_get_cputype() <= 3)
  {
    int cpufreq = 300000;
    if (limit)
      cpufreq = 600000;

    aml_set_sysfs_int("/sys/devices/system/cpu/cpu0/cpufreq/scaling_min_freq", cpufreq);
  }
#endif
}

void aml_cpufreq_max(bool limit)
{
  if (!aml_wired_present() && aml_get_cputype() > 3)
  {
    // this is a MX Stick, they cannot substain 1GHz
    // operation without overheating so limit them to 800MHz.
    int cpufreq = 1000000;
    if (limit)
      cpufreq = 800000;

    aml_set_sysfs_int("/sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq", cpufreq);
    aml_set_sysfs_str("/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor", "ondemand");
  }
}

void aml_set_audio_passthrough(bool passthrough)
{
  if (aml_present())
  {
    // m1 uses 1, m3 and above uses 2
    int raw = aml_get_cputype() < 3 ? 1:2;
    aml_set_sysfs_int("/sys/class/audiodsp/digital_raw", passthrough ? raw:0);
  }
}

void aml_probe_hdmi_audio()
{
  std::vector<CStdString> audio_formats;
  // Audio {format, channel, freq, cce}
  // {1, 7, 7f, 7}
  // {7, 5, 1e, 0}
  // {2, 5, 7, 0}
  // {11, 7, 7e, 1}
  // {10, 7, 6, 0}
  // {12, 7, 7e, 0}

  int fd = open("/sys/class/amhdmitx/amhdmitx0/edid", O_RDONLY);
  if (fd >= 0)
  {
    char valstr[1024] = {0};

    read(fd, valstr, sizeof(valstr) - 1);
    valstr[strlen(valstr)] = '\0';
    close(fd);

    std::vector<CStdString> probe_str;
    StringUtils::SplitString(valstr, "\n", probe_str);

    for (size_t i = 0; i < probe_str.size(); i++)
    {
      if (probe_str[i].find("Audio") == std::string::npos)
      {
        for (size_t j = i+1; j < probe_str.size(); j++)
        {
          if      (probe_str[i].find("{1,")  != std::string::npos)
            printf(" PCM found {1,\n");
          else if (probe_str[i].find("{2,")  != std::string::npos)
            printf(" AC3 found {2,\n");
          else if (probe_str[i].find("{3,")  != std::string::npos)
            printf(" MPEG1 found {3,\n");
          else if (probe_str[i].find("{4,")  != std::string::npos)
            printf(" MP3 found {4,\n");
          else if (probe_str[i].find("{5,")  != std::string::npos)
            printf(" MPEG2 found {5,\n");
          else if (probe_str[i].find("{6,")  != std::string::npos)
            printf(" AAC found {6,\n");
          else if (probe_str[i].find("{7,")  != std::string::npos)
            printf(" DTS found {7,\n");
          else if (probe_str[i].find("{8,")  != std::string::npos)
            printf(" ATRAC found {8,\n");
          else if (probe_str[i].find("{9,")  != std::string::npos)
            printf(" One_Bit_Audio found {9,\n");
          else if (probe_str[i].find("{10,") != std::string::npos)
            printf(" Dolby found {10,\n");
          else if (probe_str[i].find("{11,") != std::string::npos)
            printf(" DTS_HD found {11,\n");
          else if (probe_str[i].find("{12,") != std::string::npos)
            printf(" MAT found {12,\n");
          else if (probe_str[i].find("{13,") != std::string::npos)
            printf(" ATRAC found {13,\n");
          else if (probe_str[i].find("{14,") != std::string::npos)
            printf(" WMA found {14,\n");
          else
            break;
        }
        break;
      }
    }
  }
}
