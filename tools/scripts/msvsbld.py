#############################################################################
# $Id$

from datetime import datetime
from fnmatch import fnmatch
import os, shutil, stat, sys, tempfile, _winreg

def buildVC2005Express(dir):
	key = _winreg.OpenKey(_winreg.HKEY_LOCAL_MACHINE, 'Software\\Microsoft\\VCExpress\\8.0\\Setup\\VC')
	vcdir = _winreg.QueryValueEx(key, 'ProductDir')[0]
	_winreg.CloseKey(key)
	tmpscript = tempfile.mktemp() + os.extsep + 'bat'
	script = open(tmpscript, 'w')
	print >> script, "call \"%s\"" % os.path.join(vcdir, 'bin' + os.sep + 'vcvars32.bat')
	dxdir = os.environ['DXSDK_DIR']
	if dxdir:
		print >> script, "set INCLUDE=%%INCLUDE%%;\"%s\"" % os.path.join(dxdir, 'Include')
		print >> script, "set LIB=%%LIB%%;\"%s\"" % os.path.join(dxdir, 'Lib' + os.sep + 'x86')
	print >> script, "pushd %s" % os.path.join(dir, 'vcproj' + os.sep + 'vc8')
	for i in ['Opt', 'Debug', 'Release', 'StaticDebug', 'StaticRelease']:
		print >> script, "vcexpress 3rdparty.sln /useenv /build %s" % i
	for i in ['Opt', 'Debug', 'Release']:
		print >> script, "vcexpress sge.sln /useenv /build %s" % i
	for i in ['StaticDebug', 'StaticRelease']:
		print >> script, "vcexpress sgestatic.sln /useenv /build %s" % i
	print >> script, 'popd'
	script.close()
	os.system(tmpscript)
	os.remove(tmpscript)

"""
function buildinstallers(bldroot)
   local nsis = autobld:getregistryvalue(HKEY_LOCAL_MACHINE, "Software\\NSIS");
   if nsis then
      enumbuilddirs(bldroot, function(baseDir, buildDir, buildCompiler)
         local shipDir = pathcat(baseDir, "Ship." .. buildCompiler);
         autobld:mkdir(shipDir);
         for i,targetDir in ipairs(autobld:dir(pathcat(baseDir, buildDir, "*"))) do
            local shipTargetDir = pathcat(shipDir, targetDir);
            autobld:mkdir(shipTargetDir);
            local buildBinDir = pathcat(baseDir, buildDir, targetDir);

            local instArgs = "/nocd ";
            instArgs = instArgs .. " /DBUILD_ROOT=" .. baseDir;
            instArgs = instArgs .. " /DBUILD_OUTDIR=" .. shipTargetDir;
            instArgs = instArgs .. " /DBUILD_BINARIES=" .. buildBinDir;
            instArgs = instArgs .. " /DBUILD_" .. string.upper(buildCompiler);
            instArgs = instArgs .. " /DBUILD_" .. string.upper(targetDir);

            local instCmd = string.format("%q", pathcat(nsis, "makensis.exe"));
            instCmd = instCmd .. " " .. "/O" .. pathcat(shipTargetDir, "makensis.log");
            instCmd = instCmd .. " " .. instArgs;
            instCmd = instCmd .. " " .. pathcat(baseDir, "installer\\sge.nsi");
            autobld:spawn(instCmd);
         end
      end);
   end
end
"""

def buildVC71(dir):
	key = _winreg.OpenKey(_winreg.HKEY_LOCAL_MACHINE, 'Software\\Microsoft\\VisualStudio\\7.1\\Setup\\VC')
	vcdir = _winreg.QueryValueEx(key, 'ProductDir')[0]
	_winreg.CloseKey(key)
	tmpscript = tempfile.mktemp() + os.extsep + 'bat'
	script = open(tmpscript, 'w')
	print >> script, "call \"%s\"" % os.path.join(vcdir, 'bin' + os.sep + 'vcvars32.bat')
	dxdir = os.environ['DXSDK_DIR']
	if dxdir:
		print >> script, "set INCLUDE=%%INCLUDE%%;\"%s\"" % os.path.join(dxdir, 'Include')
		print >> script, "set LIB=%%LIB%%;\"%s\"" % os.path.join(dxdir, 'Lib' + os.sep + 'x86')
	print >> script, "pushd %s" % os.path.join(dir, 'vcproj' + os.sep + 'vc71')
	for i in ['Opt', 'Debug', 'Release', 'StaticDebug', 'StaticRelease']:
		print >> script, "devenv 3rdparty.sln /useenv /build %s" % i
	for i in ['Opt', 'Debug', 'Release']:
		print >> script, "devenv sge.sln /useenv /build %s" % i
	for i in ['StaticDebug', 'StaticRelease']:
		print >> script, "devenv sgestatic.sln /useenv /build %s" % i
	print >> script, 'popd'
	script.close()
	os.system(tmpscript)
	os.remove(tmpscript)

def getLastBuild(dir = None):
	bld = None
	dt = None
	if dir == None:
		dir = '.'
	for f in os.listdir(dir):
		if os.path.isdir(f) and fnmatch(f, 'SGE-*-*'):
			t = os.stat(f)[stat.ST_CTIME]
			if dt == None or t > dt:
				bld = f
				dt = t
	return bld

def getLatestRevision(repos):
	tmpdir = tempfile.mktemp()
	os.system("svn co %s/trunk/sge/tools/scripts %s -q" % (repos, tmpdir))
	rev = os.popen("svnversion %s -n" % tmpdir).read()
	try:
		shutil.rmtree(tmpdir)
	except OSError:
		print >> sys.stderr, "Warning: unable to remove %s" % tmpdir
	return rev

def main(argv):
		
	reposUrl = 'svn://dpcvs.no-ip.org'
	
	try:
		strRev = getLatestRevision(reposUrl)
		rev = int(strRev)
	except ValueError:
		print >> sys.stderr, "Error: unable to determine revision number from %s" % strRev
		return -1
		
	try:
		lastBld = getLastBuild()
		if not lastBld is None:
			lastBldRev = int(lastBld.split('-')[-1])
			if lastBldRev == rev:
				print "No changes since last build (revision %d)" % rev
				return 0
	except ValueError:
		lastBld = None
	
	now = datetime.now()
	bldlabel = "SGE-%04d%02d%02d-%d" % (now.year, now.month, now.day, rev)
	
	os.system("svn export -r %d %s/trunk/sge %s" % (rev, reposUrl, bldlabel))

	buildVC71(bldlabel)
	buildVC2005Express(bldlabel)
	
	return 0
	
if __name__ == "__main__":
	sys.exit(main(sys.argv[1:]))
	