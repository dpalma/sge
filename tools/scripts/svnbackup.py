#############################################################################
# $Id$

import datetime
from fnmatch import fnmatch
import os
import stat
import sys
import tarfile

#now = datetime.datetime.now()

repos = os.path.normpath(sys.argv[1].lstrip("\"").rstrip("\""))

def getCurRev(repos):
	return os.popen("svnlook youngest %s" % repos).read().rstrip('\r\n\t ')

def fullBackup(repos, name = None):
	if name == None:
		rev = getCurRev(repos)
		reposName = os.path.split(repos)[1]
		now = datetime.datetime.now()
		name = "%s%04d%02d%02d-%s%sfull" % (reposName, now.year, now.month, now.day, rev, os.extsep)
	os.system("svnadmin dump %s -q >%s" % (repos, name))

def incrementalBackup(repos, fromRev, toRev, name):
	os.system("svnadmin dump %s -r %d:%d --incremental -q >%s" % (repos, fromRev, toRev, name))

def makeArchive(baseName, members):
	tarName = baseName + '.tar.gz'
	tarFile = tarfile.open(tarName, 'w:gz')
	for m in members:
		tarFile.add(m)
	tarFile.close()

def getRevFromDumpFileName(name):
	base = os.path.splitext(name)[0]
	return base.split('-')[-1]

def findLastFullDump(dir = None):
	name = None
	dt = None
	if dir == None:
		dir = '.'
	for f in os.listdir(dir):
		if fnmatch(f, '*.full'):
			t = os.stat(f)[stat.ST_CTIME]
			if dt == None or t > dt:
				name = f
				dt = t
	return name

lastFullDumpName = findLastFullDump()

if lastFullDumpName == None:
	fullBackup(repos)
	sys.exit(0)

lastFullDumpBase = os.path.splitext(lastFullDumpName)[0].split('-')[0]
lastIncRev = int(getRevFromDumpFileName(lastFullDumpName))
incrementals = []
for f in os.listdir('.'):
	if fnmatch(f, "%s*.[0-9]" % lastFullDumpBase):
		incrementals.append(f)
		incRev = int(getRevFromDumpFileName(f))
		if incRev > lastIncRev:
			lastIncRev = incRev
if len(incrementals) >= 6:
	m = [lastFullDumpName] + incrementals
	makeArchive(lastFullDumpBase, m)
	for i in m:
		os.unlink(i)
	fullBackup(repos)
else:
	curRev = int(getCurRev(repos))
	if curRev == lastIncRev:
		print "No changes since last backup at revision %d" % curRev
		sys.exit(0)
	else:
		fromRev = lastIncRev + 1
		incrementalBackup(repos, fromRev, curRev, "%s-%d-%d.%d" % (lastFullDumpBase, fromRev, curRev, len(incrementals) + 1))
