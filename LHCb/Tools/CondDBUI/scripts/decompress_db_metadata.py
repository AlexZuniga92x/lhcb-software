#!/bin/env python

import os
import shutil
import sqlite3 as lite
import sys
import PyCintex

import xml2struct
from xml2struct import *
from tempfile import mkdtemp


def statusbar (i, nrow):
    sys.stdout.write('\r')
    sys.stdout.write("[%-50s] %d%%" %('='*int(i*50/(nrow-1)), 100*i/(nrow-1)))
    i += 1
    sys.stdout.write('\r')
    sys.stdout.flush()

def stringdecompression(ustring):
    string = ustring
    isunicode = isinstance(ustring, unicode)
    if (isunicode): string = ustring.encode('utf-8')
    ret = PyCintex.gbl.CondDBCompression.decompress(string)
    return ret                    

def main():
    # Configure the parser
    from optparse import OptionParser

    parser = OptionParser(usage = "%prog [options]",
                          description =
"""This script tries to decompress the content of a CondDB file.
The user has to provide a source
(the exact path to a SQLite file), 
the partition to modify (DDDB, LHCBCOND or SIMCOND). The destination
for the decompressed db file (*_decompressed.db) is the current directory by default.""") 
    parser.add_option("-s", "--source",
            dest="dbpath", type="string",
            help="source db file to be decompressed")
    parser.add_option("-p", "--partition",
            dest="dbpart", type="string",
            help="Partition name in the source db file")
    parser.add_option("-d", "--dir", dest="destdir", type = "string",
            help = "Directory where to put the decompressed db file. [default is current directory]", 
            default="." )

    (options, args) = parser.parse_args()
    if len(args) != 0 or not options.dbpath or not options.dbpart:
        parser.error("Bad arguments, use -h for help.")
    dbpath = options.dbpath
    dbpart = options.dbpart
    destdir = options.destdir
    newdb = os.path.basename(dbpath)
    tmpdir = mkdtemp()
    newdb = os.path.splitext(newdb)[0] + '_decompressed' + os.path.splitext(newdb)[1]
    tmppath = os.path.join(tmpdir,newdb)
    sys.stdout.write("Copying .db file to temporary directory....\n")
    shutil.copyfile(dbpath, tmppath)
    con = None
    con = lite.connect(tmppath)
    con.text_factory = str # see http://docs.python.org/2/library/sqlite3.html#sqlite3-Connection-Objects to evade coding issues

    origsize = 0
    count = 0
    compcount = 0
    compsize = 0
    uncompsize = 0

#    Method = options.method

    with con:
        cur = con.cursor()
        cur.execute("select FOLDER_IOVTABLENAME, FOLDER_PAYLOADSPEC from %s_NODES where NODE_ISLEAF" %dbpart)
        rows = cur.fetchall()

        sys.stdout.write("Scanning for data....\n")
        nrow = len(rows)
        contents = []
        for row in rows:
            iovtblname = row[0]
            plspec = row[1]
            for pln in plspec.split(','):
                pln = pln.split(':')[0]
                cur.execute("select %s, OBJECT_ID from %s" %(pln, iovtblname))
                xmls = cur.fetchall()
                for (xml, pid) in xmls:
                    if ( not xml ): continue # do nothing in case of empty entry
                    count += 1
                    origsize += sys.getsizeof(xml)
                    contents.append((iovtblname, pln, xml, pid))

        sys.stdout.write("Start decompressing....\n")
        i = 0
        newcontents = []
        for (iovtblname, pln, xml, pid) in contents:
            newxml = stringdecompression(xml)
            compsize += sys.getsizeof(newxml)
            if (newxml==''):
                sys.stderr.write("Error during decompressing %s with %s and %s!" %(xml, pln, iovtblname))
                sys.exit(1)
            if (newxml != xml):
                compcount += 1
                newcontents.append((iovtblname, pln, newxml, pid))
#                print "Compression ratio: %d/%d" %(len(xml),len(newxml))
            else: uncompsize += sys.getsizeof(xml)
            statusbar(i, len(contents))
            i += 1

        sys.stdout.write("\nStart updating the database....\n")
        i = 0
        for (iovtblname, pln, newxml, pid) in newcontents:
#            print  "update %s set %s=\'%s\' where OBJECT_ID=%d" %(iovtblname, pln, newxml, pid) 
#            cur.execute('''update "%s" set "%s"="%s" where OBJECT_ID=%d''' %(iovtblname, pln, newxml, pid))
            cur.execute("update %s set %s = ? where OBJECT_ID == ?" %(iovtblname, pln), (newxml, pid))
            con.commit()
            statusbar(i, len(newcontents))
            i += 1
#            print "Number of rows updated: %d" % cur.rowcount

        cur.close()
        # Compact the database
        sys.stdout.write("\nCompacting the database....\n")
        con.execute("VACUUM")
    con.close()
    print "Total size / number of data entries in the .db file: %d / %d\n" %(origsize, count)
    print "Total size after decompression: %d\n" %compsize
    print "Total number of data entries decompressed: %d\n" %compcount
    print "Total size of data entries uncompressed: %d\n" %uncompsize
    sys.stdout.write("Copying compressed .db file....\n")
    print "cp " + tmppath + " " + destdir
    shutil.copy(tmppath, destdir)

if __name__ == '__main__':
    main()
