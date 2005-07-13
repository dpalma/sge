#############################################################################
# $Id$

import os

env = Environment(ENV = os.environ)

opts = Options()
opts.AddOptions(BoolOption('debug', 'Build with debugging enabled', 0))
opts.AddOptions(BoolOption('unicode', 'Build with _UNICODE defined', 0))
opts.Update(env)

Help("Usage: scons [debug] [unicode]" + opts.GenerateHelpText(env))

########################################

platform = str(Platform())

if platform == 'win32':
   glIncludePaths = ['#3rdparty/Cg/include']
   glLibPaths = ['#3rdparty/Cg/lib']
   glLibs = ['opengl32.lib', 'glu32.lib', 'Cg.lib', 'CgGL.lib']
elif platform == 'cygwin':
   glIncludePaths = ['/usr/include', '/usr/X11R6/include']
   glLibPaths = ['/usr/lib', '/usr/X11R6/lib']
   glLibs = ['GL', 'GLU', 'X11']
else:
   print 'Unsupported platform'
   Exit(1)

########################################

if env.get('unicode'):
   env.Append(CPPDEFINES=['_UNICODE', 'UNICODE'])
else:
   env.Append(CPPDEFINES=['_MBCS'])

if platform == 'win32':
   env.Append(CCFLAGS=['/GX', '/FD'], CPPDEFINES=['_WIN32', 'WIN32', 'STRICT']);
   env.Append(CPPDEFINES=['TIXML_USE_STL', 'GLEW_STATIC'])
   if env.get('debug'):
      env.Append(CCFLAGS=['/MTd', '/Od', '/GZ'], CPPDEFINES=['DEBUG', '_DEBUG'])
   else:
      env.Append(CCFLAGS=['/MT', '/O2'], CPPDEFINES=['NDEBUG'])
      env.Append(LINKFLAGS=['/opt:ref'])
elif platform == 'cygwin':
   if env.get('debug'):
      env.Append(CCFLAGS=['-g'])
   else:
      env.Append(CCFLAGS=['-o3'])
else:
   print 'Unsupported platform'
   Exit(1)

########################################

modules = Split("""
   3rdparty
   allguids
   tech
   render
   engine
   game
""")

if platform == 'win32':
   modules = modules + ['MilkShapeExporter', 'editor']

Export('env glIncludePaths glLibPaths glLibs')
SConscript(dirs = modules)
