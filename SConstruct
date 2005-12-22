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
#         CCFLAGS.remove('/MDd')
#         CCFLAGS.remove('/MD')
#         CCFLAGS.remove('/MTd')
#         CCFLAGS.remove('/MT')
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
      self.__PreBuild(*args, **kw)
      self.StaticLibrary(*args, **kw)

   def BuildSharedLibrary(self, *args, **kw):
      self.__PreBuild(*args, **kw)
      self.SharedLibrary(*args, **kw)

   def BuildLibrary(self, *args, **kw):
#      if 'deffile' in kw:
#         sources.append(kw.pop('deffile'))
      if self.shared:
         self.BuildSharedLibrary(*args, **kw)
      else:
         self.BuildStaticLibrary(*args, **kw)
         
   def BuildExecutable(self, *args, **kw):
      self.__PreBuild(*args, **kw)
      self.Program(*args, **kw)
      
   def AdjustLibDir(self, libDir):
      if libDir[0] == '#':
         libDir = libDir.lstrip("#")
      target = GetTargetNameFromDir(libDir)
      return "#" + os.path.join(self.GetBuildDir(), target)

   def GetBuildDir(self):
      if self.IsDebug():
         return "Build.Debug"
      else:
         return "Build.Release"


#############################################################################

opts = Options(None, ARGUMENTS)
opts.AddOptions(
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
   return target

########################################

sconscripts = CollectTargets()

for script in sconscripts:

   # Backslashes must be escaped twice, once for re and once for Python itself
   script = re.sub(re.escape(re.escape(os.getcwd())), '', re.escape(script))

   # Get rid of leading backslash if present since script is a relative path now
   # The os.path.normpath call converts double-backslashes back to single
   script = re.sub(r'^\\{1,2}', '', os.path.normpath(script))
   
   dir, scriptOnly = os.path.split(script)
   
   target = GetTargetNameFromDir(dir)
   targetDir = os.path.join(env.GetBuildDir(), target)
   
   BuildDir(targetDir, dir, duplicate=0)
   SConscript(os.path.join(targetDir, scriptOnly), exports='env')
   env.Alias(target, targetDir)
