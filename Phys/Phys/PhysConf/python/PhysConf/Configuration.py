"""
High level configuration tools for PhysConf
"""
__version__ = "$Id: Configuration.py,v 1.17 2009-09-02 16:42:06 jpalac Exp $"
__author__  = "Patrick Koppenburg <Patrick.Koppenburg@cern.ch>"

from LHCbKernel.Configuration import *
from GaudiConf.Configuration import *
from Configurables import GaudiSequencer
from Configurables import LoKiSvc
import GaudiKernel.ProcessJobOptions

from Configurables import OffLineCaloRecoConf  
from Configurables import OffLineCaloPIDsConf   


class PhysConf(LHCbConfigurableUser) :
    from Configurables import (GaudiSequencer)
    __slots__ = {
        "DataType"        : 'MC09'                             # Data type, can be ['DC06','2008']
     ,  "Simulation"      : True                               # set to True to use SimCond
     ,  "InputType"       : 'DST'                              # Hopefully irrelevant
        }
    
    __used_configurables__ = (
        OffLineCaloRecoConf ,
        OffLineCaloPIDsConf 
        )
    
#
# configure reconstruction to be redone
#
    def initSequence(self):
        """
        Init Sequence. Called by master application.
        """
        # only one initialisiation do far
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
        ## CaloPIDs 
        caloPIDs = OffLineCaloPIDsConf (
            EnablePIDsOnDemand = True   , ## enable PIDs-On-Demand
            DataType           = self.getProp('DataType')
            )
        ## Calo reco
        caloReco = OffLineCaloRecoConf (
            EnableRecoOnDemand = True     ## enable Reco-On-Demand
            )
        #
        # Proto recalibration
        #
        from Configurables import (GaudiSequencer)
        # Test protos are here
        testrecalib = GaudiSequencer("TestProtoPRecalibration")
        init.Members += [ testrecalib ]
        from Configurables import (TESCheck)
        protoCheck = TESCheck("CheckProto")
        protoCheck.Inputs = [ "Rec/ProtoP" ] 
        protoCheck.Stop = False 
        testrecalib.Members += [ protoCheck ]
        # Do recalibration
        recalib = GaudiSequencer("ProtoPRecalibration")
        recalib.IgnoreFilterPassed = True 
        testrecalib.Members += [ recalib ]
        inputtype =  self.getProp('InputType').upper()
        if ( inputtype != 'RDST' ) :
            log.info('Doing MuonID for '+self.getProp( "DataType" )+' and '+inputtype)
            from MuonID import ConfiguredMuonIDs
            from Configurables import MuonRec, ChargedProtoParticleAddMuonInfo, ChargedProtoCombineDLLsAlg
            cm=ConfiguredMuonIDs.ConfiguredMuonIDs(data=self.getProp("DataType"))
            MuonIDSeq=cm.getMuonIDSeq()
            recalib.Members += [ MuonRec(), MuonIDSeq,
                                 ChargedProtoParticleAddMuonInfo(), ChargedProtoCombineDLLsAlg() ]
        else:
            # @todo Should use DoD Svc, but there are some problems
            from Configurables import (MuonPIDsFromProtoParticlesAlg)
            recalib.Members += [ MuonPIDsFromProtoParticlesAlg("MuonPIDsFromProtos") ]
        from Configurables import (RichPIDsFromProtoParticlesAlg)
        recalib.Members += [ RichPIDsFromProtoParticlesAlg("RichPIDsFromProtos") ]
    
#
# Data on demand
#
    def dataOnDemand(self):
        """
        dataOnDemand service
        """
        DataOnDemandSvc().NodeMap['/Event/Rec']            = 'DataObject'
        DataOnDemandSvc().NodeMap['/Event/Rec/Muon']       = 'DataObject'
        DataOnDemandSvc().NodeMap['/Event/Rec/Rich']       = 'DataObject'
        DataOnDemandSvc().NodeMap['/Event/Phys']           = 'DataObject'
        DataOnDemandSvc().NodeMap['/Event/Relations/Phys'] = 'DataObject'
        # raw event, not for DC06
        importOptions("$STDOPTS/DecodeRawEvent.py")
#
# LoKi
#
    def loki(self) :
        """
        Define loki service
        """
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
#        self.configureReco() # call it directly
        self.loki()
        
