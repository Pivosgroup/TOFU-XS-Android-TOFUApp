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

#include "LangInfo.h"
#include "Application.h"
#include "ApplicationMessenger.h"
#include "FileItem.h"
#include "Util.h"
#include "filesystem/Directory.h"
#include "guilib/GUIFontManager.h"
#include "guilib/LocalizeStrings.h"
#include "pvr/PVRManager.h"
#include "settings/AdvancedSettings.h"
#include "settings/lib/Setting.h"
#include "settings/Settings.h"
#include "utils/CharsetConverter.h"
#include "utils/log.h"
#include "utils/LangCodeExpander.h"
#include "utils/StringUtils.h"
#include "utils/Weather.h"
#include "utils/XBMCTinyXML.h"

using namespace std;
using namespace PVR;

#define TEMP_UNIT_STRINGS 20027

#define SPEED_UNIT_STRINGS 20200

CLangInfo::CRegion::CRegion(const CRegion& region)
{
  m_strName=region.m_strName;
  m_forceUnicodeFont=region.m_forceUnicodeFont;
  m_strGuiCharSet=region.m_strGuiCharSet;
  m_strSubtitleCharSet=region.m_strSubtitleCharSet;
  m_strDVDMenuLanguage=region.m_strDVDMenuLanguage;
  m_strDVDAudioLanguage=region.m_strDVDAudioLanguage;
  m_strDVDSubtitleLanguage=region.m_strDVDSubtitleLanguage;
  m_strLangLocaleName = region.m_strLangLocaleName;
  m_strLangLocaleCodeTwoChar = region.m_strLangLocaleCodeTwoChar;
  m_strRegionLocaleName = region.m_strRegionLocaleName;

  m_strDateFormatShort=region.m_strDateFormatShort;
  m_strDateFormatLong=region.m_strDateFormatLong;
  m_strTimeFormat=region.m_strTimeFormat;
  m_strMeridiemSymbols[MERIDIEM_SYMBOL_PM]=region.m_strMeridiemSymbols[MERIDIEM_SYMBOL_PM];
  m_strMeridiemSymbols[MERIDIEM_SYMBOL_AM]=region.m_strMeridiemSymbols[MERIDIEM_SYMBOL_AM];
  m_strTimeFormat=region.m_strTimeFormat;
  m_tempUnit=region.m_tempUnit;
  m_speedUnit=region.m_speedUnit;
  m_strTimeZone = region.m_strTimeZone;
}

CLangInfo::CRegion::CRegion()
{
  SetDefaults();
}

CLangInfo::CRegion::~CRegion()
{

}

void CLangInfo::CRegion::SetDefaults()
{
  m_strName="N/A";
  m_forceUnicodeFont=false;
  m_strGuiCharSet="CP1252";
  m_strSubtitleCharSet="CP1252";
  m_strDVDMenuLanguage="en";
  m_strDVDAudioLanguage="en";
  m_strDVDSubtitleLanguage="en";
  m_strLangLocaleName = "English";
  m_strLangLocaleCodeTwoChar = "en";

  m_strDateFormatShort="DD/MM/YYYY";
  m_strDateFormatLong="DDDD, D MMMM YYYY";
  m_strTimeFormat="HH:mm:ss";
  m_tempUnit=TEMP_UNIT_CELSIUS;
  m_speedUnit=SPEED_UNIT_KMH;
  m_strTimeZone.clear();
}

void CLangInfo::CRegion::SetTempUnit(const CStdString& strUnit)
{
  if (strUnit.Equals("F"))
    m_tempUnit=TEMP_UNIT_FAHRENHEIT;
  else if (strUnit.Equals("K"))
    m_tempUnit=TEMP_UNIT_KELVIN;
  else if (strUnit.Equals("C"))
    m_tempUnit=TEMP_UNIT_CELSIUS;
  else if (strUnit.Equals("Re"))
    m_tempUnit=TEMP_UNIT_REAUMUR;
  else if (strUnit.Equals("Ra"))
    m_tempUnit=TEMP_UNIT_RANKINE;
  else if (strUnit.Equals("Ro"))
    m_tempUnit=TEMP_UNIT_ROMER;
  else if (strUnit.Equals("De"))
    m_tempUnit=TEMP_UNIT_DELISLE;
  else if (strUnit.Equals("N"))
    m_tempUnit=TEMP_UNIT_NEWTON;
}

void CLangInfo::CRegion::SetSpeedUnit(const CStdString& strUnit)
{
  if (strUnit.Equals("kmh"))
    m_speedUnit=SPEED_UNIT_KMH;
  else if (strUnit.Equals("mpmin"))
    m_speedUnit=SPEED_UNIT_MPMIN;
  else if (strUnit.Equals("mps"))
    m_speedUnit=SPEED_UNIT_MPS;
  else if (strUnit.Equals("fth"))
    m_speedUnit=SPEED_UNIT_FTH;
  else if (strUnit.Equals("ftm"))
    m_speedUnit=SPEED_UNIT_FTMIN;
  else if (strUnit.Equals("fts"))
    m_speedUnit=SPEED_UNIT_FTS;
  else if (strUnit.Equals("mph"))
    m_speedUnit=SPEED_UNIT_MPH;
  else if (strUnit.Equals("kts"))
    m_speedUnit=SPEED_UNIT_KTS;
  else if (strUnit.Equals("beaufort"))
    m_speedUnit=SPEED_UNIT_BEAUFORT;
  else if (strUnit.Equals("inchs"))
    m_speedUnit=SPEED_UNIT_INCHPS;
  else if (strUnit.Equals("yards"))
    m_speedUnit=SPEED_UNIT_YARDPS;
  else if (strUnit.Equals("fpf"))
    m_speedUnit=SPEED_UNIT_FPF;
}

void CLangInfo::CRegion::SetTimeZone(const CStdString& strTimeZone)
{
  m_strTimeZone = strTimeZone;
}

// set the locale associated with this region global. This affects string
// sorting & transformations
void CLangInfo::CRegion::SetGlobalLocale()
{
  CStdString strLocale;
  if (m_strRegionLocaleName.length() > 0)
  {
    strLocale = m_strLangLocaleName + "_" + m_strRegionLocaleName;
#ifdef TARGET_POSIX
    strLocale += ".UTF-8";
#endif
  }

  CLog::Log(LOGDEBUG, "trying to set locale to %s", strLocale.c_str());

  // We need to set the locale to only change the collate. Otherwise,
  // decimal separator is changed depending of the current language
  // (ie. "," in French or Dutch instead of "."). This breaks atof() and
  // others similar functions.
#if defined(TARGET_FREEBSD) || defined(TARGET_DARWIN_OSX) || defined(TARGET_LINUX)
  // on FreeBSD and darwin libstdc++ is compiled with "generic" locale support
  if (setlocale(LC_COLLATE, strLocale.c_str()) == NULL
  || setlocale(LC_CTYPE, strLocale.c_str()) == NULL)
  {
    strLocale = "C";
    setlocale(LC_COLLATE, strLocale.c_str());
    setlocale(LC_CTYPE, strLocale.c_str());
  }
#else
  locale current_locale = locale::classic(); // C-Locale
  try
  {
    locale lcl = locale(strLocale);
    strLocale = lcl.name();
    current_locale = current_locale.combine< collate<wchar_t> >(lcl);
    current_locale = current_locale.combine< ctype<wchar_t> >(lcl);

    assert(use_facet< numpunct<char> >(current_locale).decimal_point() == '.');

  } catch(...) {
    current_locale = locale::classic();
    strLocale = "C";
  }

  locale::global(current_locale);
#endif
  g_charsetConverter.resetSystemCharset();
  CLog::Log(LOGINFO, "global locale set to %s", strLocale.c_str());
}

CLangInfo::CLangInfo()
{
  SetDefaults();
}

CLangInfo::~CLangInfo()
{
}

void CLangInfo::OnSettingChanged(const CSetting *setting)
{
  if (setting == NULL)
    return;

  const std::string &settingId = setting->GetId();
  if (settingId == "locale.audiolanguage")
    SetAudioLanguage(((CSettingString*)setting)->GetValue());
  else if (settingId == "locale.subtitlelanguage")
    SetSubtitleLanguage(((CSettingString*)setting)->GetValue());
  else if (settingId == "locale.language")
  {
    if (!SetLanguage(((CSettingString*)setting)->GetValue()))
      ((CSettingString*)CSettings::Get().GetSetting("locale.language"))->Reset();
  }
  else if (settingId == "locale.country")
  {
    g_langInfo.SetCurrentRegion(((CSettingString*)setting)->GetValue());
    g_weatherManager.Refresh(); // need to reset our weather, as temperatures need re-translating.
  }
}

bool CLangInfo::Load(const std::string& strFileName, bool onlyCheckLanguage /*= false*/)
{
  SetDefaults();

  CXBMCTinyXML xmlDoc;
  if (!xmlDoc.LoadFile(strFileName))
  {
    CLog::Log(onlyCheckLanguage ? LOGDEBUG : LOGERROR, "unable to load %s: %s at line %d", strFileName.c_str(), xmlDoc.ErrorDesc(), xmlDoc.ErrorRow());
    return false;
  }

  TiXmlElement* pRootElement = xmlDoc.RootElement();
  if (pRootElement->ValueStr() != "language")
  {
    CLog::Log(onlyCheckLanguage ? LOGDEBUG : LOGERROR, "%s Doesn't contain <language>", strFileName.c_str());
    return false;
  }

  if (pRootElement->Attribute("locale"))
    m_defaultRegion.m_strLangLocaleName = pRootElement->Attribute("locale");

#ifdef TARGET_WINDOWS
  // Windows need 3 chars isolang code
  if (m_defaultRegion.m_strLangLocaleName.length() == 2)
  {
    if (! g_LangCodeExpander.ConvertTwoToThreeCharCode(m_defaultRegion.m_strLangLocaleName, m_defaultRegion.m_strLangLocaleName, true))
      m_defaultRegion.m_strLangLocaleName = "";
  }

  if (!g_LangCodeExpander.ConvertWindowsToGeneralCharCode(m_defaultRegion.m_strLangLocaleName, m_languageCodeGeneral))
    m_languageCodeGeneral = "";
#else
  if (m_defaultRegion.m_strLangLocaleName.length() != 3)
  {
    if (!g_LangCodeExpander.ConvertToThreeCharCode(m_languageCodeGeneral, m_defaultRegion.m_strLangLocaleName, !onlyCheckLanguage))
      m_languageCodeGeneral = "";
  }
  else
    m_languageCodeGeneral = m_defaultRegion.m_strLangLocaleName;
#endif

  CStdString tmp;
  if (g_LangCodeExpander.ConvertToTwoCharCode(tmp, m_defaultRegion.m_strLangLocaleName))
    m_defaultRegion.m_strLangLocaleCodeTwoChar = tmp;

  const TiXmlNode *pCharSets = pRootElement->FirstChild("charsets");
  if (pCharSets && !pCharSets->NoChildren())
  {
    const TiXmlNode *pGui = pCharSets->FirstChild("gui");
    if (pGui && !pGui->NoChildren())
    {
      CStdString strForceUnicodeFont = ((TiXmlElement*) pGui)->Attribute("unicodefont");

      if (strForceUnicodeFont.Equals("true"))
        m_defaultRegion.m_forceUnicodeFont=true;

      m_defaultRegion.m_strGuiCharSet=pGui->FirstChild()->Value();
    }

    const TiXmlNode *pSubtitle = pCharSets->FirstChild("subtitle");
    if (pSubtitle && !pSubtitle->NoChildren())
      m_defaultRegion.m_strSubtitleCharSet=pSubtitle->FirstChild()->Value();
  }

  const TiXmlNode *pDVD = pRootElement->FirstChild("dvd");
  if (pDVD && !pDVD->NoChildren())
  {
    const TiXmlNode *pMenu = pDVD->FirstChild("menu");
    if (pMenu && !pMenu->NoChildren())
      m_defaultRegion.m_strDVDMenuLanguage=pMenu->FirstChild()->Value();

    const TiXmlNode *pAudio = pDVD->FirstChild("audio");
    if (pAudio && !pAudio->NoChildren())
      m_defaultRegion.m_strDVDAudioLanguage=pAudio->FirstChild()->Value();

    const TiXmlNode *pSubtitle = pDVD->FirstChild("subtitle");
    if (pSubtitle && !pSubtitle->NoChildren())
      m_defaultRegion.m_strDVDSubtitleLanguage=pSubtitle->FirstChild()->Value();
  }

  const TiXmlNode *pRegions = pRootElement->FirstChild("regions");
  if (pRegions && !pRegions->NoChildren())
  {
    const TiXmlElement *pRegion=pRegions->FirstChildElement("region");
    while (pRegion)
    {
      CRegion region(m_defaultRegion);
      region.m_strName=pRegion->Attribute("name");
      if (region.m_strName.empty())
        region.m_strName="N/A";

      if (pRegion->Attribute("locale"))
        region.m_strRegionLocaleName = pRegion->Attribute("locale");

#ifdef TARGET_WINDOWS
      // Windows need 3 chars regions code
      if (region.m_strRegionLocaleName.length() == 2)
      {
        if (! g_LangCodeExpander.ConvertLinuxToWindowsRegionCodes(region.m_strRegionLocaleName, region.m_strRegionLocaleName))
          region.m_strRegionLocaleName = "";
      }
#endif

      const TiXmlNode *pDateLong=pRegion->FirstChild("datelong");
      if (pDateLong && !pDateLong->NoChildren())
        region.m_strDateFormatLong=pDateLong->FirstChild()->Value();

      const TiXmlNode *pDateShort=pRegion->FirstChild("dateshort");
      if (pDateShort && !pDateShort->NoChildren())
        region.m_strDateFormatShort=pDateShort->FirstChild()->Value();

      const TiXmlElement *pTime=pRegion->FirstChildElement("time");
      if (pTime && !pTime->NoChildren())
      {
        region.m_strTimeFormat=pTime->FirstChild()->Value();
        region.m_strMeridiemSymbols[MERIDIEM_SYMBOL_AM]=pTime->Attribute("symbolAM");
        region.m_strMeridiemSymbols[MERIDIEM_SYMBOL_PM]=pTime->Attribute("symbolPM");
      }

      const TiXmlNode *pTempUnit=pRegion->FirstChild("tempunit");
      if (pTempUnit && !pTempUnit->NoChildren())
        region.SetTempUnit(pTempUnit->FirstChild()->Value());

      const TiXmlNode *pSpeedUnit=pRegion->FirstChild("speedunit");
      if (pSpeedUnit && !pSpeedUnit->NoChildren())
        region.SetSpeedUnit(pSpeedUnit->FirstChild()->Value());

      const TiXmlNode *pTimeZone=pRegion->FirstChild("timezone");
      if (pTimeZone && !pTimeZone->NoChildren())
        region.SetTimeZone(pTimeZone->FirstChild()->Value());

      m_regions.insert(PAIR_REGIONS(region.m_strName, region));

      pRegion=pRegion->NextSiblingElement("region");
    }

    if (!onlyCheckLanguage)
    {
      const CStdString& strName = CSettings::Get().GetString("locale.country");
      SetCurrentRegion(strName);
    }
  }
  g_charsetConverter.reinitCharsetsFromSettings();

  if (!onlyCheckLanguage)
    LoadTokens(pRootElement->FirstChild("sorttokens"), g_advancedSettings.m_vecTokens);

  return true;
}

bool CLangInfo::CheckLanguage(const std::string& language)
{
  CLangInfo li;
  return li.Load("special://xbmc/language/" + language + "/langinfo.xml", true);
}

void CLangInfo::LoadTokens(const TiXmlNode* pTokens, vector<CStdString>& vecTokens)
{
  if (pTokens && !pTokens->NoChildren())
  {
    const TiXmlElement *pToken = pTokens->FirstChildElement("token");
    while (pToken)
    {
      CStdString strSep= " ._";
      if (pToken->Attribute("separators"))
        strSep = pToken->Attribute("separators");
      if (pToken->FirstChild() && pToken->FirstChild()->Value())
      {
        if (strSep.empty())
          vecTokens.push_back(pToken->FirstChild()->Value());
        else
          for (unsigned int i=0;i<strSep.size();++i)
            vecTokens.push_back(CStdString(pToken->FirstChild()->Value())+strSep[i]);
      }
      pToken = pToken->NextSiblingElement();
    }
  }
}

void CLangInfo::SetDefaults()
{
  m_regions.clear();

  //Reset default region
  m_defaultRegion.SetDefaults();

  // Set the default region, we may be unable to load langinfo.xml
  m_currentRegion=&m_defaultRegion;
  
  m_languageCodeGeneral = "eng";
}

CStdString CLangInfo::GetGuiCharSet() const
{
  CStdString strCharSet;
  strCharSet=CSettings::Get().GetString("locale.charset");
  if (strCharSet=="DEFAULT")
    strCharSet=m_currentRegion->m_strGuiCharSet;

  return strCharSet;
}

CStdString CLangInfo::GetSubtitleCharSet() const
{
  CStdString strCharSet=CSettings::Get().GetString("subtitles.charset");
  if (strCharSet=="DEFAULT")
    strCharSet=m_currentRegion->m_strSubtitleCharSet;

  return strCharSet;
}

bool CLangInfo::SetLanguage(const std::string &strLanguage)
{
  string strLangInfoPath = StringUtils::Format("special://xbmc/language/%s/langinfo.xml", strLanguage.c_str());
  if (!Load(strLangInfoPath))
    return false;

  if (ForceUnicodeFont() && !g_fontManager.IsFontSetUnicode())
  {
    CLog::Log(LOGINFO, "Language needs a ttf font, loading first ttf font available");
    CStdString strFontSet;
    if (!g_fontManager.GetFirstFontSetUnicode(strFontSet))
      CLog::Log(LOGERROR, "No ttf font found but needed: %s", strFontSet.c_str());
  }

  if (!g_localizeStrings.Load("special://xbmc/language/", strLanguage))
    return false;

  // also tell our weather and skin to reload as these are localized
  g_weatherManager.Refresh();
  g_PVRManager.LocalizationChanged();
  CApplicationMessenger::Get().ExecBuiltIn("ReloadSkin", false);

  return true;
}

bool CLangInfo::CheckLoadLanguage(const std::string &language)
{
  return Load("special://xbmc/language/" + language + "/langinfo.xml", true);
}

// three char language code (not win32 specific)
const CStdString& CLangInfo::GetAudioLanguage() const
{
  if (!m_audioLanguage.empty())
    return m_audioLanguage;

  return m_languageCodeGeneral;
}

void CLangInfo::SetAudioLanguage(const std::string& language)
{
  if (language.empty()
    || StringUtils::EqualsNoCase(language, "default")
    || StringUtils::EqualsNoCase(language, "original")
    || !g_LangCodeExpander.ConvertToThreeCharCode(m_audioLanguage, language))
    m_audioLanguage.clear();
}

// three char language code (not win32 specific)
const CStdString& CLangInfo::GetSubtitleLanguage() const
{
  if (!m_subtitleLanguage.empty())
    return m_subtitleLanguage;

  return m_languageCodeGeneral;
}

void CLangInfo::SetSubtitleLanguage(const std::string& language)
{
  if (language.empty()
    || StringUtils::EqualsNoCase(language, "default")
    || StringUtils::EqualsNoCase(language, "original")
    || !g_LangCodeExpander.ConvertToThreeCharCode(m_subtitleLanguage, language))
    m_subtitleLanguage.clear();
}

// two character codes as defined in ISO639
const std::string CLangInfo::GetDVDMenuLanguage() const
{
  CStdString code;
  if (!g_LangCodeExpander.ConvertToTwoCharCode(code, m_currentRegion->m_strLangLocaleName))
    code = m_currentRegion->m_strDVDMenuLanguage;
  
  return code;
}

// two character codes as defined in ISO639
const std::string CLangInfo::GetDVDAudioLanguage() const
{
  CStdString code;
  if (!g_LangCodeExpander.ConvertToTwoCharCode(code, m_audioLanguage))
    code = m_currentRegion->m_strDVDAudioLanguage;
  
  return code;
}

// two character codes as defined in ISO639
const std::string CLangInfo::GetDVDSubtitleLanguage() const
{
  CStdString code;
  if (!g_LangCodeExpander.ConvertToTwoCharCode(code, m_subtitleLanguage))
    code = m_currentRegion->m_strDVDSubtitleLanguage;
  
  return code;
}

const std::string CLangInfo::GetLanguageLocale(bool twochar /* = false */) const
{
  if (twochar)
    return m_currentRegion->m_strLangLocaleCodeTwoChar;

  return m_currentRegion->m_strLangLocaleName;
}

const CStdString& CLangInfo::GetRegionLocale() const
{
  return m_currentRegion->m_strRegionLocaleName;
}

// Returns the format string for the date of the current language
const CStdString& CLangInfo::GetDateFormat(bool bLongDate/*=false*/) const
{
  if (bLongDate)
    return m_currentRegion->m_strDateFormatLong;
  else
    return m_currentRegion->m_strDateFormatShort;
}

// Returns the format string for the time of the current language
const CStdString& CLangInfo::GetTimeFormat() const
{
  return m_currentRegion->m_strTimeFormat;
}

const CStdString& CLangInfo::GetTimeZone() const
{
  return m_currentRegion->m_strTimeZone;
}

// Returns the AM/PM symbol of the current language
const CStdString& CLangInfo::GetMeridiemSymbol(MERIDIEM_SYMBOL symbol) const
{
  return m_currentRegion->m_strMeridiemSymbols[symbol];
}

// Fills the array with the region names available for this language
void CLangInfo::GetRegionNames(CStdStringArray& array)
{
  for (ITMAPREGIONS it=m_regions.begin(); it!=m_regions.end(); ++it)
  {
    CStdString strName=it->first;
    if (strName=="N/A")
      strName=g_localizeStrings.Get(416);
    array.push_back(strName);
  }
}

// Set the current region by its name, names from GetRegionNames() are valid.
// If the region is not found the first available region is set.
void CLangInfo::SetCurrentRegion(const CStdString& strName)
{
  ITMAPREGIONS it=m_regions.find(strName);
  if (it!=m_regions.end())
    m_currentRegion=&it->second;
  else if (!m_regions.empty())
    m_currentRegion=&m_regions.begin()->second;
  else
    m_currentRegion=&m_defaultRegion;

  m_currentRegion->SetGlobalLocale();
}

// Returns the current region set for this language
const CStdString& CLangInfo::GetCurrentRegion() const
{
  return m_currentRegion->m_strName;
}

CLangInfo::TEMP_UNIT CLangInfo::GetTempUnit() const
{
  return m_currentRegion->m_tempUnit;
}

// Returns the temperature unit string for the current language
const CStdString& CLangInfo::GetTempUnitString() const
{
  return g_localizeStrings.Get(TEMP_UNIT_STRINGS+m_currentRegion->m_tempUnit);
}

CLangInfo::SPEED_UNIT CLangInfo::GetSpeedUnit() const
{
  return m_currentRegion->m_speedUnit;
}

// Returns the speed unit string for the current language
const CStdString& CLangInfo::GetSpeedUnitString() const
{
  return g_localizeStrings.Get(SPEED_UNIT_STRINGS+m_currentRegion->m_speedUnit);
}

void CLangInfo::SettingOptionsLanguagesFiller(const CSetting *setting, std::vector< std::pair<std::string, std::string> > &list, std::string &current)
{
  //find languages...
  CFileItemList items;
  XFILE::CDirectory::GetDirectory("special://xbmc/language/", items);

  vector<string> vecLanguage;
  for (int i = 0; i < items.Size(); ++i)
  {
    CFileItemPtr pItem = items[i];
    if (pItem->m_bIsFolder)
    {
      if (StringUtils::EqualsNoCase(pItem->GetLabel(), ".svn") ||
          StringUtils::EqualsNoCase(pItem->GetLabel(), "fonts") ||
          StringUtils::EqualsNoCase(pItem->GetLabel(), "media"))
        continue;

      vecLanguage.push_back(pItem->GetLabel());
    }
  }

  sort(vecLanguage.begin(), vecLanguage.end(), sortstringbyname());

  for (unsigned int i = 0; i < vecLanguage.size(); ++i)
    list.push_back(make_pair(vecLanguage[i], vecLanguage[i]));
}

void CLangInfo::SettingOptionsStreamLanguagesFiller(const CSetting *setting, std::vector< std::pair<std::string, std::string> > &list, std::string &current)
{
  list.push_back(make_pair(g_localizeStrings.Get(308), "original"));
  list.push_back(make_pair(g_localizeStrings.Get(309), "default"));

  // get a list of language names
  vector<string> languages = g_LangCodeExpander.GetLanguageNames();
  sort(languages.begin(), languages.end(), sortstringbyname());
  for (std::vector<std::string>::const_iterator language = languages.begin(); language != languages.end(); ++language)
    list.push_back(make_pair(*language, *language));
}

void CLangInfo::SettingOptionsRegionsFiller(const CSetting *setting, std::vector< std::pair<std::string, std::string> > &list, std::string &current)
{
  CStdStringArray regions;
  g_langInfo.GetRegionNames(regions);
  sort(regions.begin(), regions.end(), sortstringbyname());

  bool match = false;
  for (unsigned int i = 0; i < regions.size(); ++i)
  {
    CStdString region = regions[i];
    list.push_back(make_pair(region, region));

    if (!match && region.Equals(((CSettingString*)setting)->GetValue().c_str()))
    {
      match = true;
      current = region;
    }
  }

  if (!match && regions.size() > 0)
    current = regions[0];
}
