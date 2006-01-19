#############################################################################
# $Id$

import os
import re
from Walk import Walk

#############################################################################

platform = str(Platform())

if not platform in ['win32', 'cygwin']:
   print 'Unsupported platform', platform
   Exit(1)


#############################################################################
#
# CLASS: SGEEnvironment
#

class SGEEnvironment(Environment):

   def __init__(self,**kw):
      Environment.__init__(self,**kw)
      self.debug = 1
      self.shared = 0

   def UseTinyxml(self):
      self.Append(CPPDEFINES = ['TIXML_USE_STL'])
      self.Append(CPPPATH    = ['#3rdparty/tinyxml'])
      self.Append(LIBPATH    = [self.AdjustLibDir('#3rdparty/tinyxml')])
      self.Append(LIBS       = ['tinyxml'])
      
   def UseGL(self):
      self.Append(CPPDEFINES = ['GLEW_STATIC'])
      self.Append(CPPPATH    = ['#3rdparty/glew/include'])
      self.Append(LIBPATH    = [self.AdjustLibDir('#3rdparty/glew')])
      self.Append(LIBS       = ['glew'])
      if platform == 'win32':
         self.Append(CPPPATH = ['#3rdparty/Cg/include'])
         self.Append(LIBPATH = ['#3rdparty/Cg/lib'])
         self.Append(LIBS    = ['opengl32.lib', 'glu32.lib', 'Cg.lib', 'CgGL.lib'])
      elif platform == 'cygwin':
         self.Append(CPPPATH = ['/usr/include', '/usr/X11R6/include'])
         self.Append(LIBPATH = ['/usr/lib', '/usr/X11R6/lib'])
         self.Append(LIBS    = ['GL', 'GLU', 'X11'])
         
   def UseZLib(self):
      self.Append(CPPPATH    = ['#3rdparty/zlib', '#3rdparty/zlib/contrib/minizip'])
      self.Append(LIBPATH    = [self.AdjustLibDir('#3rdparty/zlib')])
      self.Append(LIBS       = ['zlibwapi'])
      
   def UseLua(self):
      self.Append(CPPPATH    = ['#3rdparty/lua/include'])
      self.Append(LIBPATH    = [self.AdjustLibDir('#3rdparty/lua')])
      self.Append(LIBS       = ['lua'])
      
   def SetCommon(self):
      if platform == 'win32':
         self.Append(CCFLAGS=['/EHsc']);
         # HACK: turn off deprecation warnings and for-scope standards compliance for VC2005
         if self.get('MSVS_VERSION') == '8.0':
            self.Append(CXXFLAGS=['/Zc:forScope-'])
            self.Append(CPPDEFINES=['_CRT_SECURE_NO_DEPRECATE'])
            
   def IsShared(self):
      return self.shared
         
   def SetStatic(self):
      self.shared = 0
      self.Append(CPPDEFINES=['NO_AUTO_EXPORTS'])
      if platform == 'win32':
         if self.IsDebug():
            self.Append(CCFLAGS=['/MTd'])
         else:
            self.Append(CCFLAGS=['/MT'])
      
   def SetShared(self):
      self.shared = 1
      if platform == 'win32':
         if self.IsDebug():
            self.Append(CCFLAGS=['/MDd'])
         else:
            self.Append(CCFLAGS=['/MD'])
            
   def IsDebug(self):
      return self.debug
      
   def SetDebug(self):
      self.debug = 1
      self.SetCommon()
      if platform == 'win32':
         self.Append(CCFLAGS=['/Od'], CPPDEFINES=['DEBUG'])
         if self.get('MSVS_VERSION') == '8.0':
            self.Append(CCFLAGS=['/RTC1'])
         else:
            self.Append(CCFLAGS=['/GZ'])
      elif platform == 'cygwin':
         self.Append(CCFLAGS=['-g'])
         
   def SetRelease(self):
      self.debug = 0
      self.SetCommon()
      if platform == 'win32':
         self.Append(CCFLAGS=['/O2'], CPPDEFINES=['NDEBUG'], LINKFLAGS=['/OPT:REF'])
      elif platform == 'cygwin':
         self.Append(CCFLAGS=['-o3'])
         
   def __PreBuild(self, *args, **kw):
      if 'include_path' in kw:
         self.Append(CPPPATH=kw.pop('include_path'))
      if 'lib_path' in kw:
         libPaths = map(lambda x: self.AdjustLibDir(x), kw.pop('lib_path'))
         self.Append(LIBPATH=libPaths)
      if 'libs' in kw:
         self.Append(LIBS=kw.pop('libs'))
         
   def BuildStaticLibrary(self, *args, **kw):
      if 'lib_path' in kw:
         kw.pop('lib_path')
      if 'libs' in kw:
         kw.pop('libs')
      self.__PreBuild(*args, **kw)
      self.StaticLibrary(*args, **kw)

   def BuildSharedLibrary(self, *args, **kw):
      self.__PreBuild(*args, **kw)
      # add def file to sources
      if 'deffile' in kw:
         args[1].append(kw.pop('deffile'))
      if str(Platform()) == 'win32':
         self.Append(LIBS=['user32', 'kernel32', 'gdi32', 'winmm'])
      self.Append(CPPDEFINES=[args[0].upper() + '_EXPORTS'])
      self.SharedLibrary(*args, **kw)

   def BuildLibrary(self, *args, **kw):
      if self.IsShared():
         self.BuildSharedLibrary(*args, **kw)
      else:
         self.BuildStaticLibrary(*args, **kw)
         
   def BuildExecutable(self, *args, **kw):
      self.__PreBuild(*args, **kw)
      if str(Platform()) == 'win32':
         self.Append(LIBS=['user32', 'kernel32', 'gdi32', 'winmm'])
      self.Program(*args, **kw)
      
   def AdjustLibDir(self, libDir):
      if libDir[0] == '#':
         libDir = libDir.lstrip("#")
      target = GetTargetNameFromDir(libDir)
      return "#" + os.path.join(self.GetBuildDir(), target)

   def GetBuildDir(self):
      return buildRootDir


#############################################################################

opts = Options(None, ARGUMENTS)
opts.AddOptions(
   BoolOption('show_only', 'Show targets without actually building', 0),
   BoolOption('debug', 'Build with debugging enabled', 0),
   BoolOption('unicode', 'Build with _UNICODE defined', 0),
   BoolOption('shared', 'Build shared libraries', 0))

env = SGEEnvironment(ENV = os.environ, options = opts)

Help("Usage: scons [debug] [unicode]" + opts.GenerateHelpText(env))

########################################

if env.get('unicode'):
   env.Append(CPPDEFINES=['_UNICODE', 'UNICODE'])
else:
   env.Append(CPPDEFINES=['_MBCS'])
   
if env.get('debug'):
   env.SetDebug()
else:
   env.SetRelease()

if env.get('shared'):
   env.SetShared()
else:   
   env.SetStatic()
   
if env.get('debug') and env.get('shared'):
   mode = "Debug"
elif not env.get('debug') and env.get('shared'):
   mode = "Release"
elif env.get('debug') and not env.get('shared'):
   mode = "StaticDebug"
elif not env.get('debug') and not env.get('shared'):
   mode = "StaticRelease"
   
buildRootDir = 'build' + os.sep + str(Platform()) + os.sep + mode
Export('buildRootDir')

########################################

def CollectTargets():
   sconscripts = Walk(os.getcwd(), 1, 'SConscript', 0)
   # Remove Windows-specific projects if not building for Windows
   if platform != 'win32' or env.get('MSVS_VERSION') == '8.0':
      for script in sconscripts:
         if re.search('.*MilkShapeExporter|editor.*', script):
            print 'Removing Windows-specific project ', script
            sconscripts.remove(script)
   return sconscripts

########################################

def GetTargetNameFromDir(dir):
   dir, target = os.path.split(dir)
   while target in ['src', 'include', 'bin']:
      dir, target = os.path.split(dir)
   # remove trailing backslash if present
   while target.endswith('\\'):
      target = target.split('\\', 1)[0]
   return target

########################################

sconscripts = CollectTargets()

for script in sconscripts:

   # The target name is the name of the directory in which
   # the SConscript file resides
   temp = script.split(os.sep)
   target = temp[-2]
   
   sourceDir = os.path.split(script)[0]
   buildDir = os.path.join(env.GetBuildDir(), target)
   buildScript = os.path.join(buildDir, temp[-1])

   if env.get('show_only'):
      print 'Target: ' + target + ' (script is ' + buildScript + ')'
   else:
      BuildDir(buildDir, sourceDir, duplicate=0)
      SConscript(buildScript, exports='env')
      env.Alias(target, buildDir)
