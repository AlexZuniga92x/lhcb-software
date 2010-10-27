#!/usr/bin/env gaudirun.py
#
# Minimal file for running LumiFsrWriter from python prompt
# Syntax is:
#   gaudirun.py ../job/LumiFsrWriter.py
#
import os
from Gaudi.Configuration import *

#--- switch on xml summary
from Configurables import LHCbApp
LHCbApp().XMLSummary = 'summary.xml'
#-- set explicit CondDB tag
LHCbApp().CondDBtag = 'head-20100826'

#--- determine application to run
from LumiAlgs.LumiFsrWriterConf import LumiFsrWriterConf as LumiFsrWriter

#-- File catalogs. First one is read-write
FileCatalog().Catalogs = [ "xmlcatalog_file:MyCatalog.xml" ]
files = [
  "   DATAFILE='mdf:root://castorlhcb.cern.ch//castor/cern.ch/grid/lhcb/data/2010/RAW/FULL/LHCb/COLLISION10/81363/081363_0000000313.raw?svcClass=lhcbrdst' SVC='LHCb::MDFSelector'",
    ]

ofilename='testFSRout0.dst'

LumiFsrWriter().inputFiles = files
LumiFsrWriter().outputFile =  ofilename
LumiFsrWriter().EvtMax =  1000
LumiFsrWriter().OutputLevel =  INFO
EventSelector().PrintFreq = 1000

#-- to test layer uncomment these lines
# from Configurables import CondDB
# CondDB().addLayer(dbFile = "LHCBCOND_Lumi_IOV.db", dbName = "LHCBCOND")
