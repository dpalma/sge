#############################################################################
# $Id$

import os
import re

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
   env.Append(CPPDEFINES=['TIXML_USE_STL', 'GLEW_STATIC', 'NO_AUTO_EXPORTS'])
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
   print 'Unsupported platform ', platform
   Exit(1)

########################################
# From http://aspn.activestate.com/ASPN/Cookbook/Python/Recipe/52664

def Walk( root, recurse=0, pattern='*', return_folders=0 ):
	import fnmatch, os, string
	
	# initialize
	result = []

	# must have at least root folder
	try:
		names = os.listdir(root)
	except os.error:
		return result

	# expand pattern
	pattern = pattern or '*'
	pat_list = string.splitfields( pattern , ';' )
	
	# check each file
	for name in names:
		fullname = os.path.normpath(os.path.join(root, name))

		# grab if it matches our pattern and entry type
		for pat in pat_list:
			if fnmatch.fnmatch(name, pat):
				if os.path.isfile(fullname) or (return_folders and os.path.isdir(fullname)):
					result.append(fullname)
				continue
				
		# recursively scan other folders, appending results
		if recurse:
			if os.path.isdir(fullname) and not os.path.islink(fullname):
				result = result + Walk( fullname, recurse, pattern, return_folders )
			
	return result

########################################

Export('env glIncludePaths glLibPaths glLibs')

sconscripts = Walk(os.getcwd(), 1, 'SConscript', 0)

# Remove Windows-specific projects if not building for Windows
if platform != 'win32':
   for script in sconscripts:
      if re.search('.*MilkShapeExporter|editor.*', script):
         print 'Removing Windows-specific project ', script
         sconscripts.remove(script)

for script in sconscripts:
   SConscript(script)
