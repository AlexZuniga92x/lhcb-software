#! /usr/bin/env python

import CondDBUI
from PyCool import cool
import os
import datetime

def dateString():
    import datetime
    now = datetime.datetime.now()
    return now.strftime("%d%m%Y")

def getUNIXTime(dtime):
    # Note dtime must be a date in CET (CERN) time.
    import time
    t = time.mktime(dtime.timetuple())
    zone = time.tzname[0]
    if zone not in ['GMT','CET'] : raise Exception('Unknown time zone '+zone)
    offset = 0
    if zone == 'GMT' : offset = -3600
    return int( (t+offset) * 1e9 )

def genXML(root,cond):
    file = open(root+cond)
    data = ""
    for line in file :
        if -1 != line.find("../../../DTD/structure.dtd"):
            data += """<!DOCTYPE DDDB SYSTEM "conddb:/DTD/structure.dtd">"""
        else:
            data += line
    file.close()
    return data

def addToDB(startTime,stopTime,rootToFiles,condPath,db):
    #start = 0
    start = getUNIXTime(startTime) 
    #stop  = cool.ValidityKeyMax
    stop = getUNIXTime(stopTime)
    print " -> Condition", condPath, "updated"
    db.storeXMLString( condPath, genXML(rootToFiles,condPath), start, stop )

def findXMLFiles(rootdir):
    import os
    fileList = [ ]
    for root, subFolders, files in os.walk(rootdir):
        for file in files :
            splits = file.split(".")
            if len(splits) > 1 and splits[len(splits)-1] == "xml" :
                fileList.append(os.path.join(root,file))
    return fileList

def condDBPathFromFile(rootdir,fullfile):
    return fullfile.replace(rootdir,"")

def fileMD5(file):
    import hashlib
    infile = open(file,'rb')
    content = infile.read()
    infile.close()
    m = hashlib.md5()
    m.update(content)
    return m.hexdigest()

# Tracking Updates
#upAlign = "/usera/jonesc/NFS/DetDB/Tracking2011-v5.3"
#dnAlign = "/usera/jonesc/NFS/DetDB/Tracking2011-v5.3"
#dbFileName = "Tracking2011Align-v5.3"

# Mirror alignment
#upAlign    = "/usera/jonesc/NFS/DetDB/2011Mirrors/MagUp"
#dnAlign    = "/usera/jonesc/NFS/DetDB/2011Mirrors/MagDown"
#dbFileName = "2011MirrorAlign"

# Mirror alignment with new field map
#upAlign    = "/usera/jonesc/NFS/DetDB/2011MirrorsNewFieldMap/MagUp"
#dnAlign    = "/usera/jonesc/NFS/DetDB/2011MirrorsNewFieldMap/MagDown"
#dbFileName = "2011MirrorAlignNewFieldMap"

# Move HPD Occupancies
#upAlign = "/usera/jonesc/NFS/DetDB/MoveHPDOccs"
#dnAlign = "/usera/jonesc/NFS/DetDB/MoveHPDOccs"
#dbFileName = "MoveHPDOccs"

# L1 Mappings
#upAlign = "/usera/jonesc/NFS/DetDB/L1Mapping"
#dnAlign = "/usera/jonesc/NFS/DetDB/L1Mapping"
#dbFileName = "DetNumbers"

# MDMS
#upAlign = "/usera/jonesc/NFS/DetDB/MDMS"
#dnAlign = "/usera/jonesc/NFS/DetDB/MDMS"
#dbFileName = "MDMS-NoHPDOcc"

# Old HPD Occ values
#upAlign = "/usera/jonesc/NFS/DetDB/OldHPDOcc"
#dnAlign = "/usera/jonesc/NFS/DetDB/OldHPDOcc"
#dbFileName = "OldHPDOccs"

# Aero sub tiles
upAlign = "/usera/jonesc/tmp/tar_Aerogel"
dnAlign = "/usera/jonesc/tmp/tar_Aerogel"
dbFileName = "AeroSubTiles"

# The following dates are extracted from spreadsheets here
# http://marwww.in2p3.fr/~legac/LHCb/
# http://lbtriggerreport.cern.ch/reports/last_report_2011_3500_fill.html

# Hardcode the field changes. Format is date of change and the new polarity
# Dates are in CET !!
field = { }

# 2009                     Year  Month  Day   Hour  Min  Sec
#field[ datetime.datetime(  2009,   9,    1,    1,    0,   0  ) ] = dnAlign

# 2010                     Year  Month  Day   Hour  Min  Sec
field[ datetime.datetime(  2010,   4,    5,    1,    0,   0  ) ] = upAlign
#field[ datetime.datetime(  2010,   4,    6,    2,    0,   0  ) ] = dnAlign
#field[ datetime.datetime(  2010,   5,    2,   18,    0,   0  ) ] = dnAlign
#field[ datetime.datetime(  2010,   5,    3,   01,    0,   0  ) ] = upAlign
#field[ datetime.datetime(  2010,   5,    8,   20,    0,   0  ) ] = upAlign
#field[ datetime.datetime(  2010,   5,   14,   11,    0,   0  ) ] = dnAlign
#field[ datetime.datetime(  2010,   5,   15,   15,    0,   0  ) ] = upAlign
#field[ datetime.datetime(  2010,   5,   19,    4,    0,   0  ) ] = dnAlign
#field[ datetime.datetime(  2010,   7,   13,    3,    0,   0  ) ] = upAlign
#field[ datetime.datetime(  2010,   7,   28,   20,    0,   0  ) ] = dnAlign
#field[ datetime.datetime(  2010,   8,   18,    7,    0,   0  ) ] = upAlign
#field[ datetime.datetime(  2010,   8,   29,   16,    0,   0  ) ] = dnAlign
#field[ datetime.datetime(  2010,   9,   22,   15,    0,   0  ) ] = upAlign
#field[ datetime.datetime(  2010,  10,   24,    8,    0,   0  ) ] = dnAlign
# End of 2010
#stop = datetime.datetime(  2010,  12,   31,   23,   59,  59  )

# 2011                     Year  Month  Day   Hour  Min  Sec
field[ datetime.datetime(  2011,   3,    1,    1,    0,   0  ) ] = dnAlign
field[ datetime.datetime(  2011,   3,   26,   18,   20,   0  ) ] = upAlign
field[ datetime.datetime(  2011,   3,   27,   21,   15,   0  ) ] = dnAlign
field[ datetime.datetime(  2011,   4,   15,    2,   40,   0  ) ] = upAlign
field[ datetime.datetime(  2011,   4,   26,   23,   20,   0  ) ] = dnAlign
field[ datetime.datetime(  2011,   6,   11,    2,    0,   0  ) ] = upAlign

# End of 2011
#stop = datetime.datetime(  2011,  12,   31,   23,   59,  59  )

# Long way away ...
stop = datetime.datetime(  2100,  12,   31,   23,   59,  59  )

# Open a new DB file
fulDBname = dbFileName + "-" + dateString() + ".db"
if os.path.exists(fulDBname) : os.remove(fulDBname)
db = CondDBUI.CondDB( "sqlite_file:"+fulDBname+"/LHCBCOND",
                      create_new_db=True, readOnly=False )
print "Opened DB file", fulDBname

createdPaths = [ ]
lastMDsums   = { }

# Loop over the data and fill the DB slice
for start in sorted(field.keys()):
    align = field[start]
    print start, align
    
    # Find XML files
    condPaths = findXMLFiles(align)

    # Fill into the DB
    for path in condPaths :

        # Get CondDB path from file name and rootdir
        condName = condDBPathFromFile(align,path)

        # Create DB path in DB file if first time
        if condName not in createdPaths :
            print " -> Creating DB path", condName
            db.createNode(condName)
            createdPaths += [condName]

        # Check MD5 sum to previous condition
        md = fileMD5(path)
        if condName not in lastMDsums.keys() : lastMDsums[condName] = 0

        # Check if update is needed
        if md != lastMDsums[condName] :
            addToDB(start,stop,align,condName,db)
            lastMDsums[condName] = md
        else:
            print " -> Condition", condName, "same as previous. No update needed"
