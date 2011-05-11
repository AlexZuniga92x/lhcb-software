#! /usr/bin/env python

import CondDBUI
from PyCool import cool
import os, bz2
import pickle
import datetime, time
import DIRAC

def usage():
  print 'Usage: %s <name>' %(Script.scriptName)

from DIRAC.Core.Base import Script
Script.parseCommandLine()
args = Script.getPositionalArgs()
if len(args) < 1:
  usage()
  DIRAC.exit(2)

def dateString():
  import datetime
  now = datetime.datetime.now()
  return now.strftime("%d%m%Y")

def genXML(data,run):
  return """<?xml version="1.0" encoding="ISO-8859-1"?>
<!DOCTYPE DDDB SYSTEM "conddb:/DTD/structure.dtd">
<DDDB>

<!-- Run """+str(run)+""" -->

<condition name="RefractivityScaleFactor">
<param name="CurrentScaleFactor" type="double">"""+str(data)+""" </param>
</condition>

</DDDB>"""

def getUNIXTime(dtime):
  # Note dtime must be a date in CET (CERN) time.
  import time
  t = time.mktime(dtime.timetuple())
  zone = time.tzname[0]
  if zone not in ['GMT','CET'] : raise Exception('Unknown time zone '+zone)
  offset = 0
  if zone == 'GMT' : offset = -3600
  return int( (t+offset) * 1e9 )

def getCalibrationsFromFile(rad,rootName):

  # Load the Corrections
  filename = "results/"+rad+'Gas_'+rootName+'.pck.bz2'
  print "Loading (n-1) Calibrations for", rad, "from", filename
  
  # Unpickle the calibration data
  file = bz2.BZ2File(filename,"r")
  calibrations = pickle.load(file)
  
  # Close file and return the data
  file.close()
  return calibrations

def correctStartTime(run,time):
  # Runs before 72908 incorrectly did not have their secs stored, so correct by 60s
  retTime = 0
  if run < 72908 :
    startTimeOffset = int( 60 * 1e9 )
    retTime = time - startTimeOffset
  else:
    retTime = time
  return retTime

def pickleDict(filename,data):
  import pickle, os, bz2
  file = bz2.BZ2File(filename,"w")
  pickle.dump(data,file,2)
  file.close()

def loadDict(filename):
  import pickle, os, bz2
  data = { }
  if os.path.exists(filename) :
    file = bz2.BZ2File(filename,"r")
    data = pickle.load(file)
    file.close()
  return data

def fillDB(calibration,db,runsTimes,rad):

  # Create DB paths for the conditions
  path = "/Conditions/"+rad+"/Environment/Gas.xml"
  db.createNode(path)

  # Add a null entry covering all periods
  #db.storeXMLString( path, genXML(1.0,0), cool.ValidityKeyMin, cool.ValidityKeyMax )

  # Loop over runs to fill the conditions
  for run in sorted(calibration.keys()):
    scaleF = calibration[run]

    # Run IOV
    dStartTime = runsTimes["RunTimes"][run]["Start"]
    #dStopTime  = runsTimes["RunTimes"][run]["Stop"]
    #dStopTime  = runsTimes["GlobalStopTime"]
    #dStopTime  = datetime.datetime(  2010,  12,   31,   23,   59,  59  )
    dStopTime  = datetime.datetime(  2011,  12,   31,   23,   59,  59  )

    startTime = correctStartTime( run, getUNIXTime(dStartTime) )
    stopTime  = getUNIXTime( dStopTime  )
    #stopTime  = cool.ValidityKeyMax

    # Scale factor
    scale = '%g' % scaleF[0]

    # Store in the DB
    print " -> Setting", rad, "(n-1) Scale", scale, "for run", run, dStartTime, "to", dStopTime
    db.storeXMLString( path, genXML(scale,run), startTime, stopTime )

def getRunTimes(calibrations):

  times = { }
  times["RunTimes"] = { }
  times["GlobalStopTime"] = None

  runList = [ ]
  for calibration in calibrations:
    for run in sorted(calibration.keys()):
      if run not in runList : runList += [run]
  runList.sort()

  # Load the raw cached run data
  RunCacheName = "RunInfoCache.pck.bz2"
  runTimeCache = loadDict(RunCacheName)

  tmpTime = 0
  cacheUpdated = False
  for run in runList:
    # Get run start and stop times
    if run in runTimeCache.keys():
      res = runTimeCache[run]
    else:
      from LHCbDIRAC.NewBookkeepingSystem.Client.BookkeepingClient import BookkeepingClient
      res = BookkeepingClient().getRunInformations(int(run))
      if len(res.keys()) > 0 and res.has_key('OK') :
        if res['OK']:
          runTimeCache[run] = res
          cacheUpdated = True
    if res['OK'] :
      start = res['Value']['RunStart']
      stop  = res['Value']['RunEnd']
      times["RunTimes"][run] = { "Start" : start, "Stop" : stop }
      print "Run", run, "is from", start, "to", stop
      unixEndTime = getUNIXTime( stop )
      if unixEndTime > tmpTime :
        tmpTime = unixEndTime
        times["GlobalStopTime"] = stop
    else:
      print "ERROR Getting start/stop times for run", run
      DIRAC.exit(1)

  # Update the saved cache
  if cacheUpdated : pickleDict(RunCacheName,runTimeCache)

  # Return the Run Time Information
  return times

# ======================================================================================

# Name of calibrations
rootName = str(args[0])

# Open a new DB
dbFileName = "results/"+rootName+"-"+dateString()+".db"
if os.path.exists(dbFileName) : os.remove(dbFileName)
db = CondDBUI.CondDB( "sqlite_file:"+dbFileName+"/LHCBCOND",
                      create_new_db=True, readOnly=False )
print "Opened DB file", dbFileName

# Load the RICH1 and RICH2 calibrations from file
cali1 = getCalibrationsFromFile('Rich1',rootName)
cali2 = getCalibrationsFromFile('Rich2',rootName)

# Create a unique dict of all run start,stop times
runsTimes = getRunTimes([cali1,cali2])

# Fill the Conditions
print "Filling CondDB slice ...."
fillDB(cali1,db,runsTimes,'Rich1')
fillDB(cali2,db,runsTimes,'Rich2')
print "Done"

DIRAC.exit(0)

# ======================================================================================
