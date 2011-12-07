"""
High level configuration tools for PhysConf
"""
__version__ = "$Id$"
__author__  = "Patrick Koppenburg <Patrick.Koppenburg@cern.ch>"

from LHCbKernel.Configuration import *
from GaudiConf.Configuration import *
import GaudiKernel.ProcessJobOptions
from Configurables import (EventClockSvc,
                           RecEventTime,
                           TimeDecoderList,
                           OdinTimeDecoder)
def configureEventTime() :
    """
    Configure EventClockSvc to get event time from RecHeader first
    and then from ODIN in case of faulire.
    Returns EventClockSvs()
    Author: Marco Clemencic.
    """
    ecs = EventClockSvc()
    ecs.addTool(TimeDecoderList, "EventTimeDecoder")
    tdl = ecs.EventTimeDecoder
    tdl.addTool(RecEventTime)
    tdl.addTool(OdinTimeDecoder)
    tdl.Decoders = [tdl.RecEventTime, tdl.OdinTimeDecoder]
    return ecs

class PhysConf(LHCbConfigurableUser) :

    __slots__ = {
           "DataType"          : ''      # Data type
        ,  "Simulation"        : False   # set to True to use SimCond
        ,  "InputType"         : 'DST'   # Data type
        ,  "AllowPIDRerunning" : True    # Allow, under the correct circumstances, PID reconstruction to be rerun (e.g. MuonID)
        ,  "EnableUnpack"      : None    # Enable unpacking of DST.
        }

    __used_configurables__ = (
        'CaloDstUnPackConf'   ,
        'OffLineCaloRecoConf' ,
        'OffLineCaloPIDsConf' 
        )
    
#
# configure reconstruction to be redone
#
    def initSequence(self):
        """
        Init Sequence. Called by master application.
        """
        # only one initialisiation so far
        from Configurables import GaudiSequencer
        init = GaudiSequencer("PhysInitSeq")
        self.configureReco(init)
        return init
        
#
# configure reconstruction to be redone
#
    def configureReco(self,init):
        """
        Configure Reconstruction to be redone
        """

        from Configurables  import OffLineCaloRecoConf
        from Configurables  import OffLineCaloPIDsConf 

        ## CaloPIDs 
        caloPIDs = OffLineCaloPIDsConf (
            EnablePIDsOnDemand = True   , ## enable PIDs-On-Demand
            DataType           = self.getProp('DataType')
            )

        ## Calo reco

        caloReco = OffLineCaloRecoConf (
            EnableRecoOnDemand = True     ## enable Reco-On-Demand
            )

        ## General unpacking
        from Configurables import DstConf
        if self.isPropertySet('EnableUnpack') :
            DstConf().setProp('EnableUnpack',self.getProp('EnableUnpack')) 
        
        ## unpack Calo (?)
        from Configurables import CaloDstUnPackConf 
        unpack = CaloDstUnPackConf()

        if self.isPropertySet('EnableUnpack') :
            unpack.setProp('Enable', self.getProp('EnableUnpack') )

        if unpack.getProp( 'Enable' ) : 
            hypos    = unpack   .getProp( 'Hypos'   )
            reco_old = caloReco .getProp( 'RecList' )
            reco_new = [ h for h in reco_old if h not in hypos ]
            caloReco.RecList = reco_new
            log.warning("PhysConf: CaloReco.RecList is redefined: %s:" %  reco_new )

        # For backwards compatibility with MC09, we need the following to rerun
        # the Muon Reco on old data. To be removed AS SOON as this backwards compatibility
        # is no longer needed
        inputtype = self.getProp('InputType').upper()
        if ( self.getProp("DataType") == 'MC09' and inputtype != 'MDST' and
             self.getProp("AllowPIDRerunning") and inputtype != 'RDST' ) :
            
            from Configurables import DataObjectVersionFilter, MuonRec, TESCheck
            from MuonID import ConfiguredMuonIDs
            
            rerunPIDSeq = GaudiSequencer("ReRunMuonPID")
            init.Members += [rerunPIDSeq]
            
            # Check data version, to see if this is needed or not
            rerunPIDSeq.Members += [ DataObjectVersionFilter( "MuonPIDVersionCheck",
                                                              DataObjectLocation = "/Event/Rec/Muon/MuonPID",
                                                              MaxVersion = 0 ) ]
            # Check raw event is available
            rerunPIDSeq.Members += [ TESCheck( "TESCheckRawEvent",
                                               Inputs = ["DAQ/RawEvent"],
                                               Stop = False ) ]
            # Run Muon PID
            cm = ConfiguredMuonIDs.ConfiguredMuonIDs(data=self.getProp("DataType"))
            rerunPIDSeq.Members += [ MuonRec(), cm.getMuonIDSeq() ]

            # If muon PID has rerun, need to re make the Combined DLLS...
            from Configurables import ( ChargedProtoParticleAddMuonInfo,
                                        ChargedProtoCombineDLLsAlg )
            rerunPIDSeq.Members += [ ChargedProtoParticleAddMuonInfo("CProtoPAddNewMuon"),
                                     ChargedProtoCombineDLLsAlg("CProtoPCombDLLNewMuon") ]

        # Compatibility with pre-2011 data, where Rec/Summary and Trigger/RawEvent are missing
        import PhysConf.CheckMissingTESData as DataCheck
        DataCheck.checkForMissingData()

#
# Data on demand
#
    def dataOnDemand(self):
        """
        dataOnDemand service
        """
        dataOnDemand = DataOnDemandSvc()
        
        dataOnDemand.NodeMap['/Event/Rec']            = 'DataObject'
        dataOnDemand.NodeMap['/Event/Rec/Muon']       = 'DataObject'
        dataOnDemand.NodeMap['/Event/Rec/Rich']       = 'DataObject'
        dataOnDemand.NodeMap['/Event/Phys']           = 'DataObject'
        dataOnDemand.NodeMap['/Event/Relations/Phys'] = 'DataObject'
        
        # raw event
        importOptions("$STDOPTS/DecodeRawEvent.py")
            
#
# LoKi
#
    def loki(self) :
        """
        Define loki service
        """
        from Configurables import LoKiSvc
        lokiService = LoKiSvc()
        ApplicationMgr().ExtSvc +=  [ lokiService ]
        
#
# Main configuration
#
    def __apply_configuration__(self):
        """
        Apply configuration for Phys
        """
        log.info("Applying Phys configuration")
        self.dataOnDemand()
        self.loki()
        if 'DST' in  self.getProp('InputType').upper() :
            log.info('PhysConf: Configuring EventClockSvc to use RecHeader')
            configureEventTime()
