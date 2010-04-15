"""
Configuration tools for the L0 trigger
"""

from Gaudi.Configuration import *
from Configurables import GaudiSequencer
from Configurables import LHCbConfigurableUser

from L0Algs import L0CaloFromRawAlgName , emulateL0Calo   , decodeL0Calo , monitorL0Calo
from L0Algs import L0MuonFromRawAlgName , emulateL0Muon   , decodeL0Muon , monitorL0Muon
from L0Algs import L0DUFromRawAlgName   , emulateL0DU     , decodeL0DU   , monitorL0DU
from L0Algs import                        emulateL0PileUp

## @class L0Conf
#  Configurable for the L0 trigger (simulation, emulation, decoding, monitoring and filtering)
#  @author Julien Cogan <cogan@cppm.in2p3.fr>
#  @date   18/02/2009
class L0Conf(LHCbConfigurableUser) :

    __slots__ = {
        # Properties
         "ReplaceL0BanksWithEmulated" : False
        ,"SimulateL0"     : False
        ,"EmulateL0"      : False
        ,"DecodeL0"       : False
        ,"FilterL0"       : False
        ,"DecodeL0DU"     : False
        ,"FilterL0FromRaw": False
        ,"MonitorL0"      : False
        ,"TCK"            : None
        ,"EnableL0DecodingOnDemand" : False
        ,"FastL0DUDecoding"         : False
        ,"FullL0MuonDecoding"       : False
        ,"IgnoreL0MuonCondDB"       : False
        ,"L0DecodingContext"        : None
        ,"L0EmulatorContext"        : None
        ,"verbose"        : False 
        # Sequencers 
        ,"L0Sequencer"    : None 
        ,"LinkSequencer"  : None 
        ,"MoniSequencer"  : None 
        ,"FilterSequencer": None 
        ,"ETCSequencer"   : None
        # Output file
        ,"ETCOutput"      : "L0ETC.root"
        ,"DataType"       : ""
        }

    _propertyDocDct = {
        # Properties
         "ReplaceL0BanksWithEmulated" : """ If True, run the emulators and replace the existing L0 banks."""
        ,"SimulateL0"     : """ If True, run the L0 simulation and write L0Banks."""
        ,"EmulateL0"      : """ If True, run the L0 emulators and write on TES at a non default location."""
        ,"DecodeL0"       : """ If True, run the L0 decoding (decode all L0 banks)."""
        ,"DecodeL0DU"     : """ If True, run the L0DU decoding."""
        ,"MonitorL0"      : """ If True, run the L0 monitoring algorithms."""
        ,"FilterL0FromRaw": """ If True, run the L0DU decoding and filter according to L0 decision."""
        ,"FilterL0"       : """ If True, filter according to L0 decision."""
        ,"TCK"            : """ Specifies the TCK to be used in simulation or emulation."""
        ,"EnableL0DecodingOnDemand" : """If True, setup the data on demand service for L0."""
        ,"FastL0DUDecoding"         : """ If True, activate fast decoding for L0DU."""
        ,"FullL0MuonDecoding"       : """ If True, decode all the L0Muon banks. Otherwise, decode only the one with the L0MuonCandidates."""
        ,"L0DecodingContext"        : "String appended to the default TES location where the results of the decoding are stored." 
        ,"L0EmulatorContext"        : "String appended to the default TES location where the results of the emulation are stored." 
        ,"verbose"        : """Obsolete"""
        # Sequencers 
        ,"L0Sequencer"    : """ Sequencer filled according to the L0Conf properties."""
        ,"LinkSequencer"  : """ Sequencer filled with the MC associator algorithms (not configurable)."""
        ,"MoniSequencer"  : """ Sequencer filled with the L0 monitoring sequence (not configurable)."""
        ,"FilterSequencer": """ Sequencer filled with the L0Filter algorithm (not configurable)."""
        ,"ETCSequencer"   : """ Sequencer filled with the algorithm and stream to write out a L0-ETC."""
        ,"ETCOutput"      : """ Name of ETC output file."""
        ,"DataType"       : """ Data type, used to set up default TCK """
         }

    def checkOptions(self):
        """
        Sanity checks.
        """
        if self.getProp("SimulateL0") and self.getProp("EmulateL0"):
            raise L0ConfError("SimulateL0","EmulateL0","Simulation and Emulation can not run in the same job.")
        if self.getProp("ReplaceL0BanksWithEmulated") and self.getProp("SimulateL0"):
            raise L0ConfError("ReplaceL0BanksWithEmulated","SimulateL0")
        if self.getProp("ReplaceL0BanksWithEmulated") and self.getProp("EmulateL0"):
            raise L0ConfError("ReplaceL0BanksWithEmulated","EmulateL0")
        if self.getProp("DecodeL0") and self.getProp("DecodeL0DU"):
            raise L0ConfError("DecodeL0","DecodeL0DU")
        if self.getProp("FilterL0FromRaw") and self.getProp("FilterL0"):
            raise L0ConfError("FilterL0FromRaw","FilterL0")
        if self.getProp("FilterL0FromRaw") and self.getProp("DecodeL0"):
            raise L0ConfError("FilterL0FromRaw","DecodeL0")
        if self.getProp("FilterL0FromRaw") and self.getProp("DecodeL0DU"):
            raise L0ConfError("FilterL0FromRaw","DecodeL0DU")

    def l0decodingSeq(self, name="L0FromRawSeq" , writeOnTes=None ):
        """ Return a Gaudi Sequencer with the algorithms to decode the L0Calo, L0Muon and L0DU data. """
        l0decodingSeq = GaudiSequencer( name )

        # L0Calo, L0Muon and L0DU decoding algorithms
        l0calo = decodeL0Calo()
        l0muon = decodeL0Muon()
        l0du   = decodeL0DU()
        
        # Write on TES
        if writeOnTes is not None:     
            l0calo.WriteOnTES = writeOnTes
            l0muon.WriteOnTES = writeOnTes
            l0du.WriteOnTES   = writeOnTes

        # Build the sequence 
        l0decodingSeq.Members+=[ l0calo, l0muon, l0du ]
        
        return l0decodingSeq

    def l0emulatorSeq(self, name="L0EmulatorSeq", writeBanks=None, writeOnTes=None ):
        """ Return a Gaudi Sequencer with the algorithms to run the L0Calo, L0Muon, L0PileUp and L0DU emulators. """
        l0emulatorSeq = GaudiSequencer( name )

        # L0Calo, L0Muon, L0PileUp and L0DU emulating algorithms
        l0calo   = emulateL0Calo()
        l0muon   = emulateL0Muon()
        l0pileup = emulateL0PileUp()
        l0du     = emulateL0DU()

        # CondDB
        l0muon.IgnoreCondDB = self.getProp("IgnoreL0MuonCondDB")
        
        # Raw banks
        if writeBanks is not None:
            l0calo.WriteBanks = writeBanks
            l0muon.WriteBanks = writeBanks
            l0du.WriteBanks   = writeBanks
            if self.getProp("DataType").upper() in ["2009"]:
                l0du.BankVersion = 1
                
        # Write on TES
        if writeOnTes is not None:     
            l0calo.WriteOnTES = writeOnTes
            l0muon.WriteOnTES = writeOnTes
            l0du.WriteOnTES   = writeOnTes

        # Build the sequence in two steps :
        # First :  run L0Calo + L0Muon + PUVeto emulators
        l0processorSeq = GaudiSequencer( "sub"+name )
        l0processorSeq.Members+=[ l0calo, l0muon, l0pileup ]
        l0emulatorSeq.Members+=[ l0processorSeq ]
        # Second : run L0DU emulator
        l0emulatorSeq.Members+=[l0du]
        
        return l0emulatorSeq

    def l0monitoringSeq(self, name="L0MoniSeq"):
        """ Return a Gaudi Sequencer with the algorithms to decode the L0Calo, L0Muon and L0DU data. """
        l0monitoringSeq = GaudiSequencer( name )

        # Build the sequence 
        l0monitoringSeq.Members+=[ monitorL0Calo(), monitorL0Muon(), monitorL0DU() ]
        
        return l0monitoringSeq

    def _defineL0Sequencer(self):
        """
        Fill the sequencer given to the L0Sequencer attribute.
        The filling of the sequencer is done according to the L0Conf properties. 
        """
        if self.isPropertySet("L0Sequencer"):

            importOptions("$L0TCK/L0DUConfig.opts")
            # Set up the TCK to use
            l0du = emulateL0DU()
            if self.isPropertySet("TCK"):     # Use L0Conf.TCK if set
                tck = self.getProp("TCK")
                if l0du.isPropertySet("TCK"):
                    log.warning("L0DU.TCK and L0Conf().TCK both set, using L0Conf().TCK = %s"%tck)
                l0du.TCK = tck
            else:
                if l0du.isPropertySet("TCK"): # Use already defined TCK
                    log.info("L0DU.TCK was set independently of L0Conf")
                else:                         # Use default TCK
                    if self.getProp("DataType").upper() in ["DC06"]:
                        l0du.TCK = "0xFFF8"
                    elif self.getProp("DataType").upper() in ["2009"]:
                        l0du.TCK = "0x1309"
                    elif self.getProp("DataType").upper() in ["2010"]:
                        l0du.TCK = "0x1810"
                    else:
                        l0du.TCK = "0xDC09"
            log.info("L0DUAlg will use TCK=%s"%l0du.getProp('TCK'))
            
            seq=self.getProp("L0Sequencer")

            if self.getProp("ReplaceL0BanksWithEmulated"):
                replacebanksSeq = GaudiSequencer("L0DUBankSwap")
                from Configurables import bankKiller 
                removebanks=bankKiller( "RemoveL0Banks" )
                removebanks.BankTypes = ["L0DU", "L0Calo", "L0Muon", "L0MuonProcCand", "L0MuonProcData" ]
                replacebanksSeq.Members+= [ removebanks, self.l0emulatorSeq( writeBanks=True, writeOnTes=False ) ]
                
                seq.Members += [ replacebanksSeq ]
                log.warning("\n  \t** EXISTING L0 BANKS WILL BE REMOVED AND REPLACED BY EMULATED BANKS **\n\n")

            if self.getProp("SimulateL0"):
                l0simulationSeq = GaudiSequencer( "L0SimulationSeq" )

                # Pus specific processing : fill raw from MC
                from Configurables import PuVetoFillRawBuffer
                l0simulationSeq.Members+=[PuVetoFillRawBuffer()]

                # Run emulators (L0Calo + L0Muon + PUVeto + L0DU)
                l0simulationSeq.Members+=[ self.l0emulatorSeq( writeBanks=True, writeOnTes=True ) ]

                seq.Members+= [l0simulationSeq ]

            if self.getProp("DecodeL0DU"):
                seq.Members+= [ decodeL0DU() ]

            if self.getProp("DecodeL0"):
                seq.Members+= [ self.l0decodingSeq( writeOnTes=True ) ]

            if self.getProp("EmulateL0"):
                if not self.isPropertySet("L0EmulatorContext"):
                    self.setProp("L0EmulatorContext" , "Emulator")
                seq.Members+= [ self.l0emulatorSeq( writeBanks=False, writeOnTes=True ) ]

            if self.getProp("MonitorL0"):
                seq.Members+= [ self.l0monitoringSeq( ) ]
                
            if self.getProp("FilterL0FromRaw"):
                from Configurables import L0Filter
                seq.Members+= [ decodeL0DU() , L0Filter()]
                
            if self.getProp("FilterL0"):
                from Configurables import L0Filter
                seq.Members+= [ L0Filter() ]
                
    def _defineL0LinkSequencer(self):                                                        
        """
        Fill the sequencer given to the LinkSequencer attribute (not configurable).
        This sequencer is always filled with 2 L0CaloToMCParticleAsct algorithms.
        """
        if self.isPropertySet("LinkSequencer"):
            from Configurables import L0CaloToMCParticleAsct
            seq=self.getProp("LinkSequencer")
            seq.Members += [ L0CaloToMCParticleAsct() ]
            seq.Members += [ L0CaloToMCParticleAsct("L0CaloFullTruth", InputContainer = "Trig/L0/FullCalo") ]

    def _defineL0MoniSequence(self):
        """
        Fill the sequencer given to the MoniSequencer attribute (not configurable).
        This sequencer is always filled with the L0MoniSeq sequencer.
        """
        if self.isPropertySet("MoniSequencer"):
            seq=self.getProp("MoniSequencer")
            seq.Members+= [ self.l0monitoringSeq( "L0MoniSeq" ) ]

    def _defineL0FilterSequence(self):
        """
        Fill the sequencer given to the FilterSequencer attribute (not configurable).
        This sequencer is always filled with the L0Filter algorithm.
        """
        if self.isPropertySet("FilterSequencer"):
            from Configurables import L0Filter
            seq=self.getProp("FilterSequencer")
            seq.Members+= [ L0Filter() ]

    def _defineETC(self):
        """
        Import the option file for writing a L0-ETC.
        """
        if self.isPropertySet("ETCSequencer"):
            from Configurables import L0ETC,TagCollectionSvc,TagCollectionStream,EvtCollectionStream
            seq=self.getProp("ETCSequencer")
            writeTagSeq= Sequencer("SeqWriteTag")
            seq.Members+= [ writeTagSeq ]
            
            evtTupleSvc = TagCollectionSvc("EvtTupleSvc")
            # evtTupleSvc.Output = []

            ApplicationMgr().ExtSvc  += [ evtTupleSvc ]

            tagCreator=L0ETC("TagCreator")
            tagCreator.EvtColsProduce = True
            tagCreator.EvtColSplitDir = False
            tagCreator.NTupleSplitDir = False
            tagCreator.NTupleProduce  = False
            tagCreator.HistoSplitDir  = False
            tagCreator.HistoProduce   = False
            tagCreator.EvtColLUN      = "EVTTAGS"
            tagCreator.CollectionName = "EVTTAGS/TagCreator/Strip"

            tagWriter = EvtCollectionStream("TagWriter")
            tagWriter.ItemList = [ "/NTUPLES/EVTTAGS/TagCreator/1" ]
            tagWriter.EvtDataSvc = "EvtTupleSvc";
            
            MyWriter=TagCollectionStream("WR")
            MyWriter.TagCollectionSvc = evtTupleSvc
            MyWriter.ItemList =  [ "/Event#1" ]
            MyWriter.ItemList += [ "DAQ/ODIN#1" ]
            if not MyWriter.isPropertySet( "Output" ):
                MyWriter.Output = "Collection='EVTTAGS/TagCreator/1' ADDRESS='/Event' DATAFILE='" + self.getProp("ETCOutput") + "' TYP='POOL_ROOTTREE' OPT='RECREATE'"

            writeTagSeq.Members+= [ tagCreator , MyWriter ]
            
    def _setSpecificOptions(self):
        """Specific options to tune the L0 components."""

        # Fast decoding of L0DU
        if self.getProp("FastL0DUDecoding"):
            log.info("Using Fast decoding for L0DU")
            l0du   = decodeL0DU()
            from Configurables import  L0DUFromRawTool
            l0du.addTool(L0DUFromRawTool,name = "L0DUFromRawTool")
            l0du.L0DUFromRawTool.FillDataMap         = False
            l0du.L0DUFromRawTool.EncodeProcessorData = False
            l0du.L0DUFromRawTool.Emulate             = False
            l0du.L0DUFromRawTool.StatusOnTES         = False
            l0du.WriteProcData                       = False

        # Full decoding of L0Muon    
        if self.getProp("FullL0MuonDecoding"):
            log.info("Activate L0MuonProcCand and L0MuonProcData decoding")
            l0muon = decodeL0Muon()
            l0muon.DAQMode = 1

        # Set l0context for the decoding
        if self.isPropertySet("L0DecodingContext"):
            l0context = self.getProp("L0DecodingContext")
            log.info( "The results of the L0 decoding will be written at location+%s"%(l0context) )
            decodeL0Calo().L0Context = l0context
            decodeL0Muon().L0Context = l0context
            decodeL0DU().L0Context   = l0context
        
        # Set l0context for the emulation
        if self.isPropertySet("L0EmulatorContext"):
            l0context = self.getProp("L0EmulatorContext")
            log.info( "The results of the L0 emulation will be written at location+%s"%(l0context) )
            emulateL0Calo().L0Context = l0context
            emulateL0Muon().L0Context = l0context
            emulateL0DU().L0Context   = l0context
        
    def _dataOnDemand(self):
        """Configure the DataOnDemand service for L0."""
        if self.getProp("EnableL0DecodingOnDemand"):
            from Configurables import DataOnDemandSvc
            log.info("L0 on demand activated")
            DataOnDemandSvc().AlgMap["Trig/L0/MuonCtrl"]   = "L0MuonCandidatesFromRaw/"+L0MuonFromRawAlgName
            if self.getProp("FullL0MuonDecoding"):
                DataOnDemandSvc().AlgMap["Trig/L0/MuonData"]   = "L0MuonCandidatesFromRaw/"+L0MuonFromRawAlgName
            DataOnDemandSvc().AlgMap["Trig/L0/Calo"]       = "L0CaloCandidatesFromRaw/"+L0CaloFromRawAlgName
            DataOnDemandSvc().AlgMap["Trig/L0/FullCalo"]   = "L0CaloCandidatesFromRaw/"+L0CaloFromRawAlgName
            DataOnDemandSvc().AlgMap["Trig/L0/L0DUReport"] = "L0DUFromRawAlg/"+L0DUFromRawAlgName
            if not self.isPropertySet("L0Sequencer"):
                importOptions("$L0TCK/L0DUConfig.opts")

    def __apply_configuration__(self):
        """
        L0Conf configuration.
        """
        log.info("Applying L0Conf")
        self.checkOptions()
        self._defineL0Sequencer()
        self._defineL0LinkSequencer()
        self._defineL0MoniSequence()
        self._defineL0FilterSequence()
        self._defineETC()
        self._setSpecificOptions()
        self._dataOnDemand()

class L0ConfError(Exception):
    """ Raised when conflicting options have been selected in L0Conf."""
    def __init__(self,prop1,prop2,message=''):
        self._prop1=prop1
        self._prop2=prop2
        self._message=message
    def __str__(self):
        s="Conflict in L0Conf options : %s and %s are not compatible. %s\n" % (self._prop1,self._prop2,self._message)
        return s

