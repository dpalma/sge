#############################################################################
# $Id$

from getopt import getopt
import os, sys, zipfile
from Walk import Walk
from zipfile import *

def usage():
	print sys.argv[0] + ' --dir (Directory to zip) [--zip (Name of the zip file)] [--help]'
	
def main(argv):
	try:
		opts, args = getopt(argv, "hd:z:", ["help", "dir=", "zip="])
	except GetoptError:
		usage()
		return -1
		
	dir = None
	zipName = None
	
	for opt, arg in opts:
		if opt in ("-h", "--help"):
			usage()
			return 0
		elif opt in ("-d", "--dir"):
			dir = arg
		elif opt in ("-z", "--zip"):
			zipName = arg
		else:
			print >> sys.stderr, 'Unknown argument: ' + opt
			return -1
	
	if dir is None:
		usage()
		return -1
	
	if not os.path.isdir(dir):
		print >> sys.stderr, dir + ' is not a directory'
		return -1
	
	dir = os.path.normpath(dir)
	
	if zipName is None:
		zipName = dir + os.extsep + 'zip'
	z = ZipFile(zipName, 'w', ZIP_DEFLATED)
	
	files = Walk(dir, 1)
	for f in files:
		if f.find('.svn') < 0:
			if f.startswith(dir):
				aname = f.replace(dir, '').lstrip(os.sep)
				z.write(f, aname)
				
	z.close()
	
	return 0
	
if __name__ == "__main__":
	sys.exit(main(sys.argv[1:]))
