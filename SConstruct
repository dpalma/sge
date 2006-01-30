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
      self.m_buildRoot = os.getcwd()
      self.m_libs = []
      self.m_libPaths = []
      self.m_incPaths = []
      self.m_defines = []

   def UseTinyxml(self):
      self.m_libs += ['tinyxml']
      self.m_libPaths += [MakeLibPath('tinyxml')]
      self.m_defines += ['TIXML_USE_STL']
      self.m_incPaths += ['#3rdparty/tinyxml']
      
   def UseCg(self):
      self.m_libs += ['Cg', 'CgGL']
      self.m_libPaths += ['#3rdparty/Cg/lib']
      self.m_incPaths += ['#3rdparty/Cg/include']
      
   def UseGL(self):
      if platform == 'win32':
         self.Append(LIBS    = ['opengl32', 'glu32'])
      elif platform == 'cygwin':
         self.Append(CPPPATH = ['/usr/include', '/usr/X11R6/include'])
         self.Append(LIBPATH = ['/usr/lib', '/usr/X11R6/lib'])
         self.Append(LIBS    = ['GL', 'GLU', 'Xext', 'Xt', 'SM', 'ICE', 'X11', 'm'])
      self.Append(LIBS = ['glew'])
      self.Append(LIBPATH = [MakeLibPath('glew')])
      self.Append(CPPDEFINES = ['GLEW_STATIC'])
      self.Append(CPPPATH = ['#3rdparty/glew/include'])
         
   def UseZLib(self):
      self.m_libs += ['zlibwapi']
      self.m_libPaths += [MakeLibPath('zlib')]
      self.m_incPaths += ['#3rdparty/zlib', '#3rdparty/zlib/contrib/minizip']
      
   def UseLua(self):
      self.m_libs += ['lua']
      self.m_libPaths += [MakeLibPath('lua')]
      self.m_incPaths += ['#3rdparty/lua/include']
      
   def SetCommon(self):
      if platform == 'win32':
         self.Append(CCFLAGS=['/EHsc'])
         # HACK: turn off deprecation warnings and for-scope standards compliance for VC2005
         if self.get('MSVS_VERSION') == '8.0':
            self.Append(CXXFLAGS=['/Zc:forScope-'])
            self.Append(CPPDEFINES=['_CRT_SECURE_NO_DEPRECATE'])
      elif platform == 'cygwin':
         self.Append(CCFLAGS=['-mwindows'])
            
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
         libPaths = map(MakeLibPath, kw.pop('lib_path'))
         self.Append(LIBPATH=libPaths)
      if 'libs' in kw:
         self.Append(LIBS=kw.pop('libs'))
      self.Append(LIBS = self.m_libs)
      self.Append(LIBPATH = self.m_libPaths)
      self.Append(CPPPATH = self.m_incPaths)
      self.Append(CPPDEFINES = self.m_defines)

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
         if self.get('debug'):
            self['PDB'] = args[0] + '.pdb'
      self.Program(*args, **kw)


#############################################################################

opts = Options(None, ARGUMENTS)
opts.AddOptions(
   BoolOption('debug', 'Build with debugging enabled', 0),
   BoolOption('unicode', 'Build with _UNICODE defined', 0),
   BoolOption('shared', 'Build shared libraries', 0))

env = SGEEnvironment(ENV = os.environ, options = opts)

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
if not os.path.isdir(buildRootDir):
   os.makedirs(buildRootDir)

def MakeLibPath(path):
   return '#' + os.path.join(buildRootDir, path.lstrip('#'))

conf = Configure(env)
if conf.CheckHeader('afxwin.h', '<>', 'c++'):
   haveMFC = 1
if conf.CheckHeader('atlcore.h', '<>', 'c++'):
   haveATL = 1
#conf.CheckLib('opengl32', 'glBegin')
#conf.CheckLib('GL', 'glBegin')
#conf.CheckLib('GLU', 'gluLookAt')
env = conf.Finish()

Help("Usage: scons [debug] [unicode]" + opts.GenerateHelpText(env))

if env.get('unicode'):
   env.Append(CPPDEFINES=['_UNICODE', 'UNICODE'])
else:
   env.Append(CPPDEFINES=['_MBCS'])

########################################

SConsignFile(os.path.join(buildRootDir, '.sconsign'))

def CollectTargets():
   sconscripts = Walk(os.getcwd(), 1, 'SConscript', 0)
   for script in sconscripts:
      if re.search('.*MilkShapeExporter.*', script) and not haveATL:
         print 'Removing ATL-dependent project', script
         sconscripts.remove(script)
      elif re.search('.*editor.*', script) and not haveMFC:
         print 'Removing MFC-dependent project', script
         sconscripts.remove(script)
   return sconscripts

########################################

sconscripts = CollectTargets()

for script in sconscripts:

   # The target name is the name of the directory in which
   # the SConscript file resides
   temp = script.split(os.sep)
   target = temp[-2]
   
   sourceDir = os.path.split(script)[0]
   buildDir = os.path.join(buildRootDir, target)
   buildScript = os.path.join(buildDir, temp[-1])

   BuildDir(buildDir, sourceDir, duplicate=0)
   SConscript(buildScript, exports='env')
   env.Alias(target, buildDir)
