#############################################################################
# $Id$

import os
import re

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

   def UseTinyxml(self):
      self.Append(CPPDEFINES = ['TIXML_USE_STL'])
      self.Append(CPPPATH    = ['#3rdparty/tinyxml'])
      self.Append(LIBPATH    = ['#3rdparty/tinyxml'])
      self.Append(LIBS       = ['tinyxml'])
      
   def UseGL(self):
      self.Append(CPPDEFINES = ['GLEW_STATIC'])
      self.Append(CPPPATH    = ['#3rdparty/glew/include'])
      self.Append(LIBPATH    = ['#3rdparty/glew'])
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
      self.Append(LIBPATH    = ['#3rdparty/zlib'])
      self.Append(LIBS       = ['zlibwapi'])
      
   def UseLua(self):
      self.Append(CPPPATH    = ['#3rdparty/lua/include'])
      self.Append(LIBPATH    = ['#3rdparty/lua'])
      self.Append(LIBS       = ['lua'])


#############################################################################
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

#############################################################################

opts = Options(None, ARGUMENTS)
opts.AddOptions(
   BoolOption('debug', 'Build with debugging enabled', 0),
   BoolOption('unicode', 'Build with _UNICODE defined', 0))

env = SGEEnvironment(ENV = os.environ, options = opts)

Help("Usage: scons [debug] [unicode]" + opts.GenerateHelpText(env))

########################################

if env.get('unicode'):
   env.Append(CPPDEFINES=['_UNICODE', 'UNICODE'])
else:
   env.Append(CPPDEFINES=['_MBCS'])

if platform == 'win32':
   env.Append(CCFLAGS=['/GX', '/FD'], CPPDEFINES=['_WIN32', 'WIN32', 'STRICT']);
   env.Append(CPPDEFINES=['NO_AUTO_EXPORTS'])
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

########################################

sconscripts = Walk(os.getcwd(), 1, 'SConscript', 0)

# Remove Windows-specific projects if not building for Windows
if platform != 'win32':
   for script in sconscripts:
      if re.search('.*MilkShapeExporter|editor.*', script):
         print 'Removing Windows-specific project ', script
         sconscripts.remove(script)

for script in sconscripts:
   # Backslashes must be escaped twice, once for re and once for Python itself
   script = re.sub(re.escape(re.escape(os.getcwd())), '', re.escape(script))
   # Get rid of leading backslash if present since script is a relative path now
   # The os.path.normpath call converts double-backslashes back to single
   script = re.sub(r'^\\{1,2}', '', os.path.normpath(script))
   SConscript(script, exports='env')
