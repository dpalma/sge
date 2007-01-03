#############################################################################
# $Id$

from datetime import datetime
from getopt import getopt
import os, pysvn, stat, sys, tempfile, _winreg

def build(dir):
	vc8dir = dir + os.sep + 'vcproj' + os.sep + 'vc8'
	os.system("pushd %s && VCBuild /useenv 3rdparty.sln $ALL && popd" % vc8dir)
	for i in ['Debug', 'Release', 'Opt']:
		os.system("MSBuild %s /p:Configuration=%s" % (os.path.join(vc8dir, 'sge.sln'), i))
	for i in ['StaticDebug', 'StaticRelease']:
		os.system("MSBuild %s /p:Configuration=%s" % (os.path.join(vc8dir, 'sgestatic.sln'), i))
	
def usage():
	print "autobld.py [--help] [--label (Build Label)] [--repository (SVN Repository URL)] [--nobuild] [--nosvnexport]"

def main(argv):

	try:
		opts, args = getopt(argv, "hl:r:", ["help", "label=", "repository=", "nobuild", "nosvnexport"])
	except GetoptError:
		usage()
		return -1
		
	repos = None
	bldlabel = None
	nobuild = False
	nosvnexport = False
	
	for opt, arg in opts:
		if opt in ("-h", "--help"):
			usage()
			return 0
		elif opt in ("-l", "--label"):
			bldlabel = arg
		elif opt in ("-r", "--repository"):
			repos = arg
		elif opt in ("--nobuild"):
			nobuild = True
		elif opt in ("--nosvnexport"):
			nosvnexport = True
			
	if repos is None:
		repos = 'svn://dpcvs.no-ip.org'
		
	client = pysvn.Client()
	
	info = client.info2(repos, recurse=False)[0][1]
	if info.rev.kind != pysvn.opt_revision_kind.number:
		print >> sys.stderr, "Error: unable to determine latest revision number for %s" % repos
		return -1
		
	print "\nLatest revision of repository %s is %d\n" % (repos, info.rev.number)
	
	now = datetime.now()
	bldlabel = "SGE-%04d%02d%02d-%d" % (now.year, now.month, now.day, info.rev.number)
	
	print "\nBuild label is %s\n" % bldlabel
	
	if not os.path.exists(bldlabel) and not nosvnexport:
		client.export(repos + '/trunk/sge', bldlabel, revision=info.rev)
#	else:
#		print >> sys.stderr, "Error: a directory named %s already exists" % bldlabel
#		return -1
		
	if not nobuild:
		build(bldlabel)
	
	return 0
	
if __name__ == "__main__":
	sys.exit(main(sys.argv[1:]))
