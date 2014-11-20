/*
 *      Copyright (C) 2013 Team XBMC
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

#if (defined HAVE_CONFIG_H) && (!defined TARGET_WINDOWS)
  #include "config.h"
#endif

// python.h should always be included first before any other includes
#include <Python.h>
#include <osdefs.h>

#include "system.h"
#include "AddonPythonInvoker.h"
#include "addons/AddonVersion.h"

#define MODULE "xbmc"

#define RUNSCRIPT_PRAMBLE \
        "" \
        "import " MODULE "\n" \
        "xbmc.abortRequested = False\n" \
        "class xbmcout:\n" \
        "  def __init__(self, loglevel=" MODULE ".LOGNOTICE):\n" \
        "    self.ll=loglevel\n" \
        "  def write(self, data):\n" \
        "    " MODULE ".log(data,self.ll)\n" \
        "  def close(self):\n" \
        "    " MODULE ".log('.')\n" \
        "  def flush(self):\n" \
        "    " MODULE ".log('.')\n" \
        "import sys\n" \
        "sys.stdout = xbmcout()\n" \
        "sys.stderr = xbmcout(" MODULE ".LOGERROR)\n" \
        ""

#define RUNSCRIPT_OVERRIDE_HACK \
        "" \
        "import os\n" \
        "def getcwd_xbmc():\n" \
        "  import __main__\n" \
        "  import warnings\n" \
        "  if hasattr(__main__, \"__file__\"):\n" \
        "    warnings.warn(\"os.getcwd() currently lies to you so please use addon.getAddonInfo('path') to find the script's root directory and DO NOT make relative path accesses based on the results of 'os.getcwd.' \", DeprecationWarning, stacklevel=2)\n" \
        "    return os.path.dirname(__main__.__file__)\n" \
        "  else:\n" \
        "    return os.getcwd_original()\n" \
        "" \
        "def chdir_xbmc(dir):\n" \
        "  raise RuntimeError(\"os.chdir not supported in xbmc\")\n" \
        "" \
        "os_getcwd_original = os.getcwd\n" \
        "os.getcwd          = getcwd_xbmc\n" \
        "os.chdir_orignal   = os.chdir\n" \
        "os.chdir           = chdir_xbmc\n" \
        ""

#if defined(TARGET_ANDROID)
// we do not ship python setuptools module and it is needed by site-packages
// this adds a simple implementation of pkg_resources.resource_filename,
// others might also be needed in the future.
// note: pkg_resources_code is a string; multi-line strings are triple quoted
// and exec will strip them just like single quoted string.
#define RUNSCRIPT_SETUPTOOLS_HACK \
        "" \
        "import imp,sys\n" \
        "pkg_resources_code = \\\n" \
        "\"\"\"\n" \
        "def resource_filename(__name__,__path__):\n" \
        "  return __path__\n" \
        "\"\"\"\n" \
        "pkg_resources = imp.new_module('pkg_resources')\n" \
        "exec pkg_resources_code in pkg_resources.__dict__\n" \
        "sys.modules['pkg_resources'] = pkg_resources\n" \
        ""
#else
#define RUNSCRIPT_SETUPTOOLS_HACK
#endif

#define RUNSCRIPT_POSTSCRIPT \
        "print '-->Python Interpreter Initialized<--'\n" \
        ""

#define RUNSCRIPT_BWCOMPATIBLE \
  RUNSCRIPT_PRAMBLE RUNSCRIPT_OVERRIDE_HACK RUNSCRIPT_SETUPTOOLS_HACK RUNSCRIPT_POSTSCRIPT

#define RUNSCRIPT_COMPLIANT \
  RUNSCRIPT_PRAMBLE RUNSCRIPT_SETUPTOOLS_HACK RUNSCRIPT_POSTSCRIPT

namespace PythonBindings {
  void initModule_xbmcgui(void);
  void initModule_xbmc(void);
  void initModule_xbmcplugin(void);
  void initModule_xbmcaddon(void);
  void initModule_xbmcvfs(void);
}

using namespace std;
using namespace PythonBindings;

typedef struct
{
  const char *name;
  CPythonInvoker::PythonModuleInitialization initialization;
} PythonModule;

static PythonModule PythonModules[] =
  {
    { "xbmcgui",    initModule_xbmcgui    },
    { "xbmc",       initModule_xbmc       },
    { "xbmcplugin", initModule_xbmcplugin },
    { "xbmcaddon",  initModule_xbmcaddon  },
    { "xbmcvfs",    initModule_xbmcvfs    }
  };

#define PythonModulesSize sizeof(PythonModules) / sizeof(PythonModule)

CAddonPythonInvoker::CAddonPythonInvoker(ILanguageInvocationHandler *invocationHandler)
  : CPythonInvoker(invocationHandler)
{ }

CAddonPythonInvoker::~CAddonPythonInvoker()
{ }

std::map<std::string, CPythonInvoker::PythonModuleInitialization> CAddonPythonInvoker::getModules() const
{
  static std::map<std::string, PythonModuleInitialization> modules;
  if (modules.empty())
  {
    for (size_t i = 0; i < PythonModulesSize; i++)
      modules.insert(make_pair(PythonModules[i].name, PythonModules[i].initialization));
  }

  return modules;
}

const char* CAddonPythonInvoker::getInitializationScript() const
{
  CStdString addonVer = ADDON::GetXbmcApiVersionDependency(m_addon);
  bool bwcompatMode = (m_addon.get() == NULL ||
                       ADDON::AddonVersion(addonVer) <= ADDON::AddonVersion("1.0"));
  return bwcompatMode ? RUNSCRIPT_BWCOMPATIBLE : RUNSCRIPT_COMPLIANT;
}
