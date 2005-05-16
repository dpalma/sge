#############################################################################
# $Id$

import os

envVars = {
   'PATH' : os.environ['PATH'],
   'INCLUDE' : os.environ['INCLUDE'],
   'LIB' : os.environ['LIB']
}

#env = Environment()
env = Environment(ENV = envVars)
#env = Environment(ENV = os.environ)

opts = Options()
opts.AddOptions(BoolOption('debug', 'Build with debugging enabled', 0))
opts.Update(env)

Help("Usage: scons <debug=yes|no>" + opts.GenerateHelpText(env))

win32_modules = ''
platform = str(Platform())
if platform == 'win32':
   win32_modules = 'MilkShapeExporter editor'
   env.Append(CCFLAGS=['/GX', '/FD'], CPPDEFINES=['_WIN32', 'WIN32', '_MBCS', 'STRICT']);
   env.Append(CPPDEFINES=['TIXML_USE_STL'])
   if env.get('debug'):
      env.Append(CCFLAGS=['/MTd', '/Od', '/GZ'], CPPDEFINES=['DEBUG', '_DEBUG'])
   else:
      env.Append(CCFLAGS=['/MT', '/O2'], CPPDEFINES=['NDEBUG'])
      env.Append(LINKFLAGS=['/opt:ref'])
else:
   print 'Unsupported platform'
   Exit(1)

modules = Split("""
   3rdparty
   allguids
   tech
   render
   engine
   game
""" + win32_modules)

Export('env')
SConscript(dirs = modules)
