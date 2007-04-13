#############################################################################
# $Id$

from getopt import getopt
import os, sys, zipfile
from Walk import Walk
from zipfile import *

def usage():
	print sys.argv[0] + ' --dir (Directory to zip) [--help]'
	
def main(argv):
	try:
		opts, args = getopt(argv, "hd:", ["help", "dir="])
	except GetoptError:
		usage()
		return -1
		
	dir = None
	
	for opt, arg in opts:
		if opt in ("-h", "--help"):
			usage()
			return 0
		elif opt in ("-d", "--dir"):
			dir = arg
		else:
			print >> sys.stderr, 'Unknown argument: ' + opt
			return -1
	
	if dir is None:
		usage()
		return -1
	
	if not os.path.isdir(dir):
		print >> sys.stderr, dir + ' is not a directory'
		return -1
	
	zname = dir + os.extsep + 'zip'
	z = ZipFile(zname, 'w', ZIP_DEFLATED)
	
	files = Walk(dir, 1)
	for f in files:
		if f.find('.svn') < 0:
			if f.startswith(dir):
				l = f.split(os.sep)
				l.remove(dir)
				aname = os.sep.join(l)
				z.write(f, aname)
				
	z.close()
	
	return 0
	
if __name__ == "__main__":
	sys.exit(main(sys.argv[1:]))
