/*
 * Many concepts and protocol specification in this code are taken
 * from Shairport, by James Laird.
 *
 *      Copyright (C) 2011-2013 Team XBMC
 *      http://xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2.1, or (at your option)
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

#if !defined(TARGET_WINDOWS)
#pragma GCC diagnostic ignored "-Wwrite-strings"
#endif

#include "AirTunesServer.h"

#ifdef HAS_AIRPLAY
#include "network/AirPlayServer.h"
#endif

#ifdef HAS_AIRTUNES

#include "utils/log.h"
#include "utils/StdString.h"
#include "network/Zeroconf.h"
#include "ApplicationMessenger.h"
#include "filesystem/PipeFile.h"
#include "Application.h"
#include "cores/dvdplayer/DVDDemuxers/DVDDemuxBXA.h"
#include "filesystem/File.h"
#include "music/tags/MusicInfoTag.h"
#include "FileItem.h"
#include "GUIInfoManager.h"
#include "guilib/GUIWindowManager.h"
#include "utils/Variant.h"
#include "utils/StringUtils.h"
#include "settings/AdvancedSettings.h"
#include "settings/Settings.h"
#include "utils/EndianSwap.h"
#include "URL.h"
#include "interfaces/AnnouncementManager.h"

#include <map>
#include <string>

#define TMP_COVERART_PATH "special://temp/airtunes_album_thumb.jpg"

using namespace XFILE;
using namespace ANNOUNCEMENT;

#if defined(HAVE_LIBSHAIRPLAY)
DllLibShairplay *CAirTunesServer::m_pLibShairplay = NULL;
#else
DllLibShairport *CAirTunesServer::m_pLibShairport = NULL;
#endif
CAirTunesServer *CAirTunesServer::ServerInstance = NULL;
CStdString CAirTunesServer::m_macAddress;
std::string CAirTunesServer::m_metadata[3];
CCriticalSection CAirTunesServer::m_metadataLock;
bool CAirTunesServer::m_streamStarted = false;

//parse daap metadata - thx to project MythTV
std::map<std::string, std::string> decodeDMAP(const char *buffer, unsigned int size)
{
  std::map<std::string, std::string> result;
  unsigned int offset = 8;
  while (offset < size)
  {
    std::string tag;
    tag.append(buffer + offset, 4);
    offset += 4;
    uint32_t length = Endian_SwapBE32(*(uint32_t *)(buffer + offset));
    offset += sizeof(uint32_t);
    std::string content;
    content.append(buffer + offset, length);//possible fixme - utf8?
    offset += length;
    result[tag] = content;
  }
  return result;
}

void CAirTunesServer::RefreshMetadata()
{
  CSingleLock lock(m_metadataLock);
  MUSIC_INFO::CMusicInfoTag tag;
  if (m_metadata[0].length())
    tag.SetAlbum(m_metadata[0]);//album
  if (m_metadata[1].length())
    tag.SetTitle(m_metadata[1]);//title
  if (m_metadata[2].length())
    tag.SetArtist(m_metadata[2]);//artist
  
  CApplicationMessenger::Get().SetCurrentSongTag(tag);
}

void CAirTunesServer::RefreshCoverArt()
{
  CSingleLock lock(m_metadataLock);
  //reset to empty before setting the new one
  //else it won't get refreshed because the name didn't change
  g_infoManager.SetCurrentAlbumThumb("");
  //update the ui
  g_infoManager.SetCurrentAlbumThumb(TMP_COVERART_PATH);
  //update the ui
  CGUIMessage msg(GUI_MSG_NOTIFY_ALL,0,0,GUI_MSG_REFRESH_THUMBS);
  g_windowManager.SendThreadMessage(msg);
}

void CAirTunesServer::SetMetadataFromBuffer(const char *buffer, unsigned int size)
{

  std::map<std::string, std::string> metadata = decodeDMAP(buffer, size);
  CSingleLock lock(m_metadataLock);

  if(metadata["asal"].length())
    m_metadata[0] = metadata["asal"];//album
  if(metadata["minm"].length())    
    m_metadata[1] = metadata["minm"];//title
  if(metadata["asar"].length())    
    m_metadata[2] = metadata["asar"];//artist
  
  RefreshMetadata();
}

void CAirTunesServer::Announce(AnnouncementFlag flag, const char *sender, const char *message, const CVariant &data)
{
  if ( (flag & Player) && strcmp(sender, "xbmc") == 0)
  {
    if (strcmp(message, "OnPlay") == 0 && m_streamStarted)
    {
      RefreshMetadata();
      RefreshCoverArt();
    }
  }
}

void CAirTunesServer::SetCoverArtFromBuffer(const char *buffer, unsigned int size)
{
  XFILE::CFile tmpFile;

  if(!size)
    return;

  CSingleLock lock(m_metadataLock);
  
  if (tmpFile.OpenForWrite(TMP_COVERART_PATH, true))
  {
    int writtenBytes=0;
    writtenBytes = tmpFile.Write(buffer, size);
    tmpFile.Close();

    if(writtenBytes)
    {
      RefreshCoverArt();
    }
  }
}

#if defined(HAVE_LIBSHAIRPLAY)
#define RSA_KEY " \
-----BEGIN RSA PRIVATE KEY-----\
MIIEpQIBAAKCAQEA59dE8qLieItsH1WgjrcFRKj6eUWqi+bGLOX1HL3U3GhC/j0Qg90u3sG/1CUt\
wC5vOYvfDmFI6oSFXi5ELabWJmT2dKHzBJKa3k9ok+8t9ucRqMd6DZHJ2YCCLlDRKSKv6kDqnw4U\
wPdpOMXziC/AMj3Z/lUVX1G7WSHCAWKf1zNS1eLvqr+boEjXuBOitnZ/bDzPHrTOZz0Dew0uowxf\
/+sG+NCK3eQJVxqcaJ/vEHKIVd2M+5qL71yJQ+87X6oV3eaYvt3zWZYD6z5vYTcrtij2VZ9Zmni/\
UAaHqn9JdsBWLUEpVviYnhimNVvYFZeCXg/IdTQ+x4IRdiXNv5hEewIDAQABAoIBAQDl8Axy9XfW\
BLmkzkEiqoSwF0PsmVrPzH9KsnwLGH+QZlvjWd8SWYGN7u1507HvhF5N3drJoVU3O14nDY4TFQAa\
LlJ9VM35AApXaLyY1ERrN7u9ALKd2LUwYhM7Km539O4yUFYikE2nIPscEsA5ltpxOgUGCY7b7ez5\
NtD6nL1ZKauw7aNXmVAvmJTcuPxWmoktF3gDJKK2wxZuNGcJE0uFQEG4Z3BrWP7yoNuSK3dii2jm\
lpPHr0O/KnPQtzI3eguhe0TwUem/eYSdyzMyVx/YpwkzwtYL3sR5k0o9rKQLtvLzfAqdBxBurciz\
aaA/L0HIgAmOit1GJA2saMxTVPNhAoGBAPfgv1oeZxgxmotiCcMXFEQEWflzhWYTsXrhUIuz5jFu\
a39GLS99ZEErhLdrwj8rDDViRVJ5skOp9zFvlYAHs0xh92ji1E7V/ysnKBfsMrPkk5KSKPrnjndM\
oPdevWnVkgJ5jxFuNgxkOLMuG9i53B4yMvDTCRiIPMQ++N2iLDaRAoGBAO9v//mU8eVkQaoANf0Z\
oMjW8CN4xwWA2cSEIHkd9AfFkftuv8oyLDCG3ZAf0vrhrrtkrfa7ef+AUb69DNggq4mHQAYBp7L+\
k5DKzJrKuO0r+R0YbY9pZD1+/g9dVt91d6LQNepUE/yY2PP5CNoFmjedpLHMOPFdVgqDzDFxU8hL\
AoGBANDrr7xAJbqBjHVwIzQ4To9pb4BNeqDndk5Qe7fT3+/H1njGaC0/rXE0Qb7q5ySgnsCb3DvA\
cJyRM9SJ7OKlGt0FMSdJD5KG0XPIpAVNwgpXXH5MDJg09KHeh0kXo+QA6viFBi21y340NonnEfdf\
54PX4ZGS/Xac1UK+pLkBB+zRAoGAf0AY3H3qKS2lMEI4bzEFoHeK3G895pDaK3TFBVmD7fV0Zhov\
17fegFPMwOII8MisYm9ZfT2Z0s5Ro3s5rkt+nvLAdfC/PYPKzTLalpGSwomSNYJcB9HNMlmhkGzc\
1JnLYT4iyUyx6pcZBmCd8bD0iwY/FzcgNDaUmbX9+XDvRA0CgYEAkE7pIPlE71qvfJQgoA9em0gI\
LAuE4Pu13aKiJnfft7hIjbK+5kyb3TysZvoyDnb3HOKvInK7vXbKuU4ISgxB2bB3HcYzQMGsz1qJ\
2gG0N5hvJpzwwhbhXqFKA4zaaSrw622wDniAK5MlIE0tIAKKP4yxNGjoD2QYjhBGuhvkWKY=\
-----END RSA PRIVATE KEY-----"

void CAirTunesServer::AudioOutputFunctions::audio_set_metadata(void *cls, void *session, const void *buffer, int buflen)
{
  CAirTunesServer::SetMetadataFromBuffer((char *)buffer, buflen);
}

void CAirTunesServer::AudioOutputFunctions::audio_set_coverart(void *cls, void *session, const void *buffer, int buflen)
{
  CAirTunesServer::SetCoverArtFromBuffer((char *)buffer, buflen);
}

char *session="XBMC-AirTunes";

void* CAirTunesServer::AudioOutputFunctions::audio_init(void *cls, int bits, int channels, int samplerate)
{
  XFILE::CPipeFile *pipe=(XFILE::CPipeFile *)cls;
  pipe->OpenForWrite(XFILE::PipesManager::GetInstance().GetUniquePipeName());
  pipe->SetOpenThreashold(300);

  Demux_BXA_FmtHeader header;
  strncpy(header.fourcc, "BXA ", 4);
  header.type = BXA_PACKET_TYPE_FMT_DEMUX;
  header.bitsPerSample = bits;
  header.channels = channels;
  header.sampleRate = samplerate;
  header.durationMs = 0;

  if (pipe->Write(&header, sizeof(header)) == 0)
    return 0;

  ThreadMessage tMsg = { TMSG_MEDIA_STOP };
  CApplicationMessenger::Get().SendMessage(tMsg, true);

  CFileItem item;
  item.SetPath(pipe->GetName());
  item.SetMimeType("audio/x-xbmc-pcm");
  m_streamStarted = true;

  CApplicationMessenger::Get().PlayFile(item);

  return session;//session
}

void  CAirTunesServer::AudioOutputFunctions::audio_set_volume(void *cls, void *session, float volume)
{
  //volume from -30 - 0 - -144 means mute
  float volPercent = volume < -30.0f ? 0 : 1 - volume/-30;
#ifdef HAS_AIRPLAY
  CAirPlayServer::backupVolume();
#endif
  if (CSettings::Get().GetBool("services.airplayvolumecontrol"))
    g_application.SetVolume(volPercent, false);//non-percent volume 0.0-1.0
}

void  CAirTunesServer::AudioOutputFunctions::audio_process(void *cls, void *session, const void *buffer, int buflen)
{
  #define NUM_OF_BYTES 64
  XFILE::CPipeFile *pipe=(XFILE::CPipeFile *)cls;
  int sentBytes = 0;
  unsigned char buf[NUM_OF_BYTES];

  while (sentBytes < buflen)
  {
    int n = (buflen - sentBytes < NUM_OF_BYTES ? buflen - sentBytes : NUM_OF_BYTES);
    memcpy(buf, (char*) buffer + sentBytes, n);

    if (pipe->Write(buf, n) == 0)
      return;

    sentBytes += n;
  }
}

void  CAirTunesServer::AudioOutputFunctions::audio_flush(void *cls, void *session)
{
  XFILE::CPipeFile *pipe=(XFILE::CPipeFile *)cls;
  pipe->Flush();
}

void  CAirTunesServer::AudioOutputFunctions::audio_destroy(void *cls, void *session)
{
  XFILE::CPipeFile *pipe=(XFILE::CPipeFile *)cls;
  pipe->SetEof();
  pipe->Close();

  //fix airplay video for ios5 devices
  //on ios5 when airplaying video
  //the client first opens an airtunes stream
  //while the movie is loading
  //in that case we don't want to stop the player here
  //because this would stop the airplaying video
#ifdef HAS_AIRPLAY
  if (!CAirPlayServer::IsPlaying())
#endif
  {
    ThreadMessage tMsg = { TMSG_MEDIA_STOP };
    CApplicationMessenger::Get().SendMessage(tMsg, true);
    CLog::Log(LOGDEBUG, "AIRTUNES: AirPlay not running - stopping player");
  }
  
  m_streamStarted = false;
}

void shairplay_log(void *cls, int level, const char *msg)
{
  int xbmcLevel = LOGINFO;

  switch(level)
  {
    case RAOP_LOG_EMERG:    // system is unusable 
      xbmcLevel = LOGFATAL;
      break;
    case RAOP_LOG_ALERT:    // action must be taken immediately
    case RAOP_LOG_CRIT:     // critical conditions
      xbmcLevel = LOGSEVERE;
      break;
    case RAOP_LOG_ERR:      // error conditions
      xbmcLevel = LOGERROR;
      break;
    case RAOP_LOG_WARNING:  // warning conditions
      xbmcLevel = LOGWARNING;
      break;
    case RAOP_LOG_NOTICE:   // normal but significant condition
      xbmcLevel = LOGNOTICE;
      break;
    case RAOP_LOG_INFO:     // informational
      xbmcLevel = LOGINFO;
      break;
    case RAOP_LOG_DEBUG:    // debug-level messages
      xbmcLevel = LOGDEBUG;
      break;
    default:
      break;
  }
    CLog::Log(xbmcLevel, "AIRTUNES: %s", msg);
}

#else

struct ao_device_xbmc
{
  XFILE::CPipeFile *pipe;
};

//audio output interface
void CAirTunesServer::AudioOutputFunctions::ao_initialize(void)
{
}

void  CAirTunesServer::AudioOutputFunctions::ao_set_volume(float volume)
{
  //volume from -30 - 0 - -144 means mute
  float volPercent = volume < -30.0f ? 0 : 1 - volume/-30;
#ifdef HAS_AIRPLAY
  CAirPlayServer::backupVolume();
#endif
  if (CSettings::Get().GetBool("services.airplayvolumecontrol"))
    g_application.SetVolume(volPercent, false);//non-percent volume 0.0-1.0
}


int CAirTunesServer::AudioOutputFunctions::ao_play(ao_device *device, char *output_samples, uint32_t num_bytes)
{
  if (!device)
    return 0;

  /*if (num_bytes && g_application.m_pPlayer->HasPlayer())
    g_application.m_pPlayer->SetCaching(CACHESTATE_NONE);*///TODO

  ao_device_xbmc* device_xbmc = (ao_device_xbmc*) device;

#define NUM_OF_BYTES 64

  unsigned int sentBytes = 0;
  unsigned char buf[NUM_OF_BYTES];
  while (sentBytes < num_bytes)
  {
    int n = (num_bytes - sentBytes < NUM_OF_BYTES ? num_bytes - sentBytes : NUM_OF_BYTES);
    memcpy(buf, (char*) output_samples + sentBytes, n);

    if (device_xbmc->pipe->Write(buf, n) == 0)
      return 0;

    sentBytes += n;
  }

  return 1;
}

int CAirTunesServer::AudioOutputFunctions::ao_default_driver_id(void)
{
  return 0;
}

ao_device* CAirTunesServer::AudioOutputFunctions::ao_open_live(int driver_id, ao_sample_format *format,
    ao_option *option)
{
  ao_device_xbmc* device = new ao_device_xbmc();

  device->pipe = new XFILE::CPipeFile;
  device->pipe->OpenForWrite(XFILE::PipesManager::GetInstance().GetUniquePipeName());
  device->pipe->SetOpenThreashold(300);

  Demux_BXA_FmtHeader header;
  strncpy(header.fourcc, "BXA ", 4);
  header.type = BXA_PACKET_TYPE_FMT_DEMUX;
  header.bitsPerSample = format->bits;
  header.channels = format->channels;
  header.sampleRate = format->rate;
  header.durationMs = 0;

  if (device->pipe->Write(&header, sizeof(header)) == 0)
  {
    delete device->pipe;
    delete device;
    return 0;
  }
  
  ThreadMessage tMsg = { TMSG_MEDIA_STOP };
  CApplicationMessenger::Get().SendMessage(tMsg, true);

  CFileItem item;
  item.SetPath(device->pipe->GetName());
  item.SetMimeType("audio/x-xbmc-pcm");

  if (ao_get_option(option, "artist"))
    item.GetMusicInfoTag()->SetArtist(ao_get_option(option, "artist"));

  if (ao_get_option(option, "album"))
    item.GetMusicInfoTag()->SetAlbum(ao_get_option(option, "album"));

  if (ao_get_option(option, "name"))
    item.GetMusicInfoTag()->SetTitle(ao_get_option(option, "name"));

  m_streamStarted = true;

  CApplicationMessenger::Get().PlayFile(item);

  return (ao_device*) device;
}

int CAirTunesServer::AudioOutputFunctions::ao_close(ao_device *device)
{
  ao_device_xbmc* device_xbmc = (ao_device_xbmc*) device;
  device_xbmc->pipe->SetEof();
  device_xbmc->pipe->Close();
  delete device_xbmc->pipe;

  //fix airplay video for ios5 devices
  //on ios5 when airplaying video
  //the client first opens an airtunes stream
  //while the movie is loading
  //in that case we don't want to stop the player here
  //because this would stop the airplaying video
#ifdef HAS_AIRPLAY
  if (!CAirPlayServer::IsPlaying())
#endif
  {
    ThreadMessage tMsg = { TMSG_MEDIA_STOP };
    CApplicationMessenger::Get().SendMessage(tMsg, true);
    CLog::Log(LOGDEBUG, "AIRTUNES: AirPlay not running - stopping player");
  }

  delete device_xbmc;
  m_streamStarted = false;

  return 0;
}

void CAirTunesServer::AudioOutputFunctions::ao_set_metadata(const char *buffer, unsigned int size)
{
  CAirTunesServer::SetMetadataFromBuffer(buffer, size);
}

void CAirTunesServer::AudioOutputFunctions::ao_set_metadata_coverart(const char *buffer, unsigned int size)
{
  CAirTunesServer::SetCoverArtFromBuffer(buffer, size);
}

/* -- Device Setup/Playback/Teardown -- */
int CAirTunesServer::AudioOutputFunctions::ao_append_option(ao_option **options, const char *key, const char *value)
{
  ao_option *op, *list;

  op = (ao_option*) calloc(1,sizeof(ao_option));
  if (op == NULL) return 0;

  op->key = strdup(key);
  op->value = strdup(value?value:"");
  op->next = NULL;

  if ((list = *options) != NULL)
  {
    list = *options;
    while (list->next != NULL)
      list = list->next;
    list->next = op;
  }
  else
  {
    *options = op;
  }

  return 1;
}

void CAirTunesServer::AudioOutputFunctions::ao_free_options(ao_option *options)
{
  ao_option *rest;

  while (options != NULL)
  {
    rest = options->next;
    free(options->key);
    free(options->value);
    free(options);
    options = rest;
  }
}

char* CAirTunesServer::AudioOutputFunctions::ao_get_option(ao_option *options, const char* key)
{

  while (options != NULL)
  {
    if (strcmp(options->key, key) == 0)
      return options->value;
    options = options->next;
  }

  return NULL;
}

int shairport_log(const char* msg, size_t msgSize)
{
  if( g_advancedSettings.m_logEnableAirtunes)
  {
    CLog::Log(LOGDEBUG, "AIRTUNES: %s", msg);
  }
  return 1;
}

#endif

bool CAirTunesServer::StartServer(int port, bool nonlocal, bool usePassword, const CStdString &password/*=""*/)
{
  bool success = false;
  CStdString pw = password;
  StopServer(true);

  m_macAddress = g_application.getNetwork().GetDefaultConnectionMacAddress();
  StringUtils::Replace(m_macAddress, ":","");
  while (m_macAddress.size() < 12)
  {
    m_macAddress = "0" + m_macAddress;
  }

  if (!usePassword)
  {
    pw.clear();
  }

  ServerInstance = new CAirTunesServer(port, nonlocal);
  if (ServerInstance->Initialize(pw))
  {
#if !defined(HAVE_LIBSHAIRPLAY)
    ServerInstance->Create();
#endif
    success = true;
  }

  if (success)
  {
    CStdString appName = StringUtils::Format("%s@%s",
                                             m_macAddress.c_str(),
                                             g_infoManager.GetLabel(SYSTEM_FRIENDLY_NAME).c_str());

    std::vector<std::pair<std::string, std::string> > txt;
    txt.push_back(std::make_pair("txtvers",  "1"));
    txt.push_back(std::make_pair("cn", "0,1"));
    txt.push_back(std::make_pair("ch", "2"));
    txt.push_back(std::make_pair("ek", "1"));
    txt.push_back(std::make_pair("et", "0,1"));
    txt.push_back(std::make_pair("sv", "false"));
    txt.push_back(std::make_pair("tp",  "UDP"));
    txt.push_back(std::make_pair("sm",  "false"));
    txt.push_back(std::make_pair("ss",  "16"));
    txt.push_back(std::make_pair("sr",  "44100"));
    txt.push_back(std::make_pair("pw",  usePassword?"true":"false"));
    txt.push_back(std::make_pair("vn",  "3"));
    txt.push_back(std::make_pair("da",  "true"));
    txt.push_back(std::make_pair("vs",  "130.14"));
    txt.push_back(std::make_pair("md",  "0,1,2"));
    txt.push_back(std::make_pair("am",  "Xbmc,1"));

    CZeroconf::GetInstance()->PublishService("servers.airtunes", "_raop._tcp", appName, port, txt);
  }

  return success;
}

void CAirTunesServer::StopServer(bool bWait)
{
  if (ServerInstance)
  {
#if !defined(HAVE_LIBSHAIRPLAY)
    if (m_pLibShairport->IsLoaded())
    {
      m_pLibShairport->shairport_exit();
    }
#endif
    ServerInstance->StopThread(bWait);
    ServerInstance->Deinitialize();
    if (bWait)
    {
      delete ServerInstance;
      ServerInstance = NULL;
    }

    CZeroconf::GetInstance()->RemoveService("servers.airtunes");
  }
}

 bool CAirTunesServer::IsRunning()
 {
   if (ServerInstance == NULL)
     return false;

   return ((CThread*)ServerInstance)->IsRunning();
 }

CAirTunesServer::CAirTunesServer(int port, bool nonlocal) : CThread("AirTunesServer")
{
  m_port = port;
#if defined(HAVE_LIBSHAIRPLAY)
  m_pLibShairplay = new DllLibShairplay();
  m_pPipe         = new XFILE::CPipeFile;  
#else
  m_pLibShairport = new DllLibShairport();
#endif
  CAnnouncementManager::AddAnnouncer(this);
}

CAirTunesServer::~CAirTunesServer()
{
#if defined(HAVE_LIBSHAIRPLAY)
  if (m_pLibShairplay->IsLoaded())
  {
    m_pLibShairplay->Unload();
  }
  delete m_pLibShairplay;
  delete m_pPipe;
#else
  if (m_pLibShairport->IsLoaded())
  {
    m_pLibShairport->Unload();
  }
  delete m_pLibShairport;
#endif
  CAnnouncementManager::RemoveAnnouncer(this);
}

void CAirTunesServer::Process()
{
  m_bStop = false;

#if !defined(HAVE_LIBSHAIRPLAY)
  while (!m_bStop && m_pLibShairport->shairport_is_running())
  {
    m_pLibShairport->shairport_loop();
  }
#endif
}

bool CAirTunesServer::Initialize(const CStdString &password)
{
  bool ret = false;

  Deinitialize();

#if defined(HAVE_LIBSHAIRPLAY)
  if (m_pLibShairplay->Load())
  {

    raop_callbacks_t ao;
    ao.cls                  = m_pPipe;
    ao.audio_init           = AudioOutputFunctions::audio_init;
    ao.audio_set_volume     = AudioOutputFunctions::audio_set_volume;
    ao.audio_set_metadata   = AudioOutputFunctions::audio_set_metadata;
    ao.audio_set_coverart   = AudioOutputFunctions::audio_set_coverart;
    ao.audio_process        = AudioOutputFunctions::audio_process;
    ao.audio_flush          = AudioOutputFunctions::audio_flush;
    ao.audio_destroy        = AudioOutputFunctions::audio_destroy;
    m_pLibShairplay->EnableDelayedUnload(false);
    m_pRaop = m_pLibShairplay->raop_init(1, &ao, RSA_KEY);//1 - we handle one client at a time max
    ret = m_pRaop != NULL;    

    if(ret)
    {
      char macAdr[6];    
      unsigned short port = (unsigned short)m_port;
      
      m_pLibShairplay->raop_set_log_level(m_pRaop, RAOP_LOG_WARNING);
      if(g_advancedSettings.m_logEnableAirtunes)
      {
        m_pLibShairplay->raop_set_log_level(m_pRaop, RAOP_LOG_DEBUG);
      }

      m_pLibShairplay->raop_set_log_callback(m_pRaop, shairplay_log, NULL);

      g_application.getNetwork().GetDefaultConnectionMacAddressRaw(macAdr);

      ret = m_pLibShairplay->raop_start(m_pRaop, &port, macAdr, 6, password.c_str()) >= 0;
    }
  }

#else

  int numArgs = 3;
  CStdString hwStr;
  CStdString pwStr;
  CStdString portStr;

  hwStr = StringUtils::Format("--mac=%s", m_macAddress.c_str());
  pwStr = StringUtils::Format("--password=%s",password.c_str());
  portStr = StringUtils::Format("--server_port=%d",m_port);

  if (!password.empty())
  {
    numArgs++;
  }

  char *argv[] = { "--apname=XBMC", (char*) portStr.c_str(), (char*) hwStr.c_str(), (char *)pwStr.c_str(), NULL };

  if (m_pLibShairport->Load())
  {

    struct AudioOutput ao;
    ao.ao_initialize = AudioOutputFunctions::ao_initialize;
    ao.ao_play = AudioOutputFunctions::ao_play;
    ao.ao_default_driver_id = AudioOutputFunctions::ao_default_driver_id;
    ao.ao_open_live = AudioOutputFunctions::ao_open_live;
    ao.ao_close = AudioOutputFunctions::ao_close;
    ao.ao_append_option = AudioOutputFunctions::ao_append_option;
    ao.ao_free_options = AudioOutputFunctions::ao_free_options;
    ao.ao_get_option = AudioOutputFunctions::ao_get_option;
#ifdef HAVE_STRUCT_AUDIOOUTPUT_AO_SET_METADATA
    ao.ao_set_metadata = AudioOutputFunctions::ao_set_metadata;    
    ao.ao_set_metadata_coverart = AudioOutputFunctions::ao_set_metadata_coverart;        
#endif
#if defined(SHAIRPORT_AUDIOOUTPUT_VERSION)
#if   SHAIRPORT_AUDIOOUTPUT_VERSION >= 2
    ao.ao_set_volume = AudioOutputFunctions::ao_set_volume;
#endif
#endif
    struct printfPtr funcPtr;
    funcPtr.extprintf = shairport_log;

    m_pLibShairport->EnableDelayedUnload(false);
    m_pLibShairport->shairport_set_ao(&ao);
    m_pLibShairport->shairport_set_printf(&funcPtr);
    m_pLibShairport->shairport_main(numArgs, argv);
    ret = true;
  }
#endif
  return ret;
}

void CAirTunesServer::Deinitialize()
{
#if defined(HAVE_LIBSHAIRPLAY)
  if (m_pLibShairplay && m_pLibShairplay->IsLoaded())
  {
    m_pLibShairplay->raop_stop(m_pRaop);
    m_pLibShairplay->raop_destroy(m_pRaop);
    m_pLibShairplay->Unload();
  }
#else
  if (m_pLibShairport && m_pLibShairport->IsLoaded())
  {
    m_pLibShairport->shairport_exit();
    m_pLibShairport->Unload();
  }
#endif
}

#endif

