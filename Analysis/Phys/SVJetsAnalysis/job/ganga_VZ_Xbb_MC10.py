#===============================================================================
# #         j.peek()                                                         # #
# #         j.peek('stdout', 'tail -f')                                      # #
#===============================================================================

# Python modules
import os

DaVinciVersion   = 'v29r1'
my_file          = 'PFDaVinci.py'
thisdir          = os.path.abspath(os.path.curdir)
my_pyFile        = thisdir + "/" + my_file

#-------------------------------------------------------------------------------
j = Job( application = DaVinci( version = DaVinciVersion ) )
j.name = 'Zjet_MC_MagUp_MC10_' + DaVinciVersion
j.application.optsfile  = [ File(my_pyFile) ]
#j.application.extraopts = '''DaVinci().EvtMax = 1000
j.application.extraopts = '''DaVinci.EvtMax = -1
DaVinci.PrintFreq  = 10000
'''

'''
from Configurables import CondDB              # <<< For submition on GANGA(DIRAC)
CondDB(UseOracle = True)                     # <<< For submition on GANGA(DIRAC)
importOptions("$APPCONFIGOPTS/DisableLFC.py") # <<< For submition on GANGA(DIRAC)
'''

#j.splitter = None

#j.splitter = SplitByFiles ( filesPerJob = 6, maxFiles = -1 )
#j.outputdata    = ["DVNtuples.root", "Bd2JpsiKS.dst"] #The ntuple will be kept in the Dirac (we need to download it).

#j.splitter = SplitByFiles ( filesPerJob = 600, maxFiles = -1 )
j.splitter = DiracSplitter ( filesPerJob = 1, maxFiles = -1 , ignoremissing = True )
#j.splitter = GenericSplitter ( filesPerJob = 200, maxFiles = -1 )
#j.splitter = DiracSplitter ( filesPerJob = 1, maxFiles = -1,ignoremissing=True )


#j.outputdata    = ["JES_minbias_histos_MC.root","JES_minbias_MC.root"]
j.outputdata    = ["JEShistosMC.root","ZJESAllMC.root"]

#j.merger = None
#j.merger = RootMerger( files = ['MuHadBinc.root'], ignorefailed = True )
#j.inputsandbox  = []
#j.outputsandbox = ["D2CharmMCTrue.root"]
#-------------------------------------------------------------------------------

#-------------------------------------------------------------------------------
#help(BKQuery)
bkq_42912001 = BKQuery( dqflag = "All" , path = "", type = "Path" )
bkq_42912001.path  = "/MC/MC10/Beam3500GeV-Oct2010-MagUp-Nu2,5/Sim01/Trig0x002e002aFlagged/Reco08/Stripping12Flagged/42912001/ALLSTREAMS.DST"#
bkq_42912002 = BKQuery( dqflag = "All" , path = "", type = "Path" )
bkq_42912002.path  = "/MC/MC10/Beam3500GeV-Oct2010-MagUp-Nu2,5/Sim01/Trig0x002e002aFlagged/Reco08/Stripping12Flagged/42912002/ALLSTREAMS.DST"#
bkq_42912004 = BKQuery( dqflag = "All" , path = "", type = "Path" )
bkq_42912004.path  = "/MC/MC10/Beam3500GeV-Oct2010-MagUp-Nu2,5/Sim01/Trig0x002e002aFlagged/Reco08/Stripping12Flagged/42912004/ALLSTREAMS.DST"#
bkq_42921001 = BKQuery( dqflag = "All" , path = "", type = "Path" )
bkq_42921001.path  = "/MC/MC10/Beam3500GeV-Oct2010-MagUp-Nu2,5/Sim01/Trig0x002e002aFlagged/Reco08/Stripping12Flagged/42921001/ALLSTREAMS.DST"#
bkq_42921004 = BKQuery( dqflag = "All" , path = "", type = "Path" )
bkq_42921004.path  = "/MC/MC10/Beam3500GeV-Oct2010-MagUp-Nu2,5/Sim01/Trig0x002e002aFlagged/Reco08/Stripping12Flagged/42921004/ALLSTREAMS.DST"#

dataset_All = LHCbDataset()
dataset_All = LHCbDataset(files = [ f for f in bkq_42912001.getDataset().files ] )
dataset_All.files.extend(bkq_42912002.getDataset().files)
dataset_All.files.extend(bkq_42912004.getDataset().files)
dataset_All.files.extend(bkq_42921001.getDataset().files)
dataset_All.files.extend(bkq_42921004.getDataset().files)

dataset = LHCbDataset( files =[ f for f in dataset_All.files ] )

print '\n Total \n ', dataset.files
j.inputdata = dataset
#-------------------------------------------------------------------------------

#-------------------------------------------------------------------------------
#j.backend = Interactive()
#j.backend = Local()
#j.backend = LSF( queue = '8nm' ) # 8nm  = about 30 min HP735     express
#j.backend = LSF( queue = '1nh' ) # 1nh  = about  3 hours         short
#j.backend = LSF( queue = '8nh' ) # 8nh  = about  1 day           medium
#j.backend = LSF( queue = '1nd' ) # 1nd  = about  3 days          long
#j.backend = LSF( queue = '1nw' ) # 1nw  = about 10 days HP K260  verylong
j.backend = Dirac()
#-------------------------------------------------------------------------------

#-------------------------------------------------------------------------------
j.submit()
#-------------------------------------------------------------------------------
