#############################################################################
# $Id$

from datetime import datetime
from fnmatch import fnmatch
from getopt import getopt
import os, stat, sys, tempfile, _winreg

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

def getLastBuild(dir = '.'):
	bld = None
	dt = None
	for f in os.listdir(dir):
		if os.path.isdir(f) and fnmatch(f, 'SGE-*-*'):
			t = os.stat(f)[stat.ST_CTIME]
			if dt is None or t > dt:
				bld = f
				dt = t
	return bld

def getLatestRevision(repos):
	info = os.popen("svn info %s --non-interactive" % repos).readlines()
	for line in info:
		key, val = line.rstrip('\n').split(':', 1)
		if key == 'Revision':
			return int(val)
	return None
	
def usage():
	print "autobld.py [--help] [--label (Build Label)] [--repository (SVN Repository URL)]"

def main(argv):

	try:
		opts, args = getopt(argv, "hl:r:", ["help", "label=", "repository="])
	except GetoptError:
		usage()
		return -1
		
	repos = None
	bldlabel = None
	
	for opt, arg in opts:
		if opt in ("-h", "--help"):
			usage()
			return 0
		elif opt in ("-l", "--label"):
			bldlabel = arg
		elif opt in ("-r", "--repository"):
			repos = arg
	
	if bldlabel is None:
		if repos is None:
			repos = 'svn://dpcvs.no-ip.org'
		try:
			rev = getLatestRevision(repos)
		except ValueError:
			print >> sys.stderr, "Error: unable to determine repository revision number"
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
		os.system("svn export -r %d %s/trunk/sge %s" % (rev, repos, bldlabel))
		
	if not os.path.exists(bldlabel):
		print >> sys.stderr, "Error: directory for specified build label %s does not exist" % bldlabel
		return -1
		
	vc8dir = bldlabel + os.sep + 'vcproj' + os.sep + 'vc8' + os.sep
	
	for i in ['Debug', 'Release', 'Opt', 'StaticDebug', 'StaticRelease']:
		os.system("msbuild %s3rdparty.sln /p:Configuration=%s" % (vc8dir, i))
		sln = None
		if 'Static' in i:
			sln='sgestatic'
		else:
			sln='sge'
		os.system("MSBuild %s%s.sln /p:Configuration=%s" % (vc8dir, sln, i))
	
	return 0
	
if __name__ == "__main__":
	sys.exit(main(sys.argv[1:]))
