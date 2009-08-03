"""
 script to configure HLT2 trigger lines

 @author P. Koppenburg
 @date 2009-07-08
"""
# =============================================================================
__author__  = "P. Koppenburg Patrick.Koppenburg@cern.ch"
__version__ = "CVS Tag $Name: not supported by cvs2svn $, $Revision: 1.18 $"
# =============================================================================
from Gaudi.Configuration import *
from LHCbKernel.Configuration import *
from Configurables import GaudiSequencer as Sequence
from Hlt2Lines.Hlt2B2DXLines            import Hlt2B2DXLinesConf
from Hlt2Lines.Hlt2B2JpsiXLines         import Hlt2B2JpsiXLinesConf
from Hlt2Lines.Hlt2B2JpsiXLines         import Hlt2Bs2JpsiPhiPrescaledAndDetachedLinesConf 
from Hlt2Lines.Hlt2B2JpsiXLines         import Hlt2Bs2JpsiPhiLinesConf
from Hlt2Lines.Hlt2B2JpsiXLines         import Hlt2Bd2JpsiKstarLinesConf
from Hlt2Lines.Hlt2B2JpsiXLines         import Hlt2Bu2JpsiKLinesConf
from Hlt2Lines.Hlt2B2PhiXLines          import Hlt2B2PhiXLinesConf
from Hlt2Lines.Hlt2InclusiveDiMuonLines import Hlt2InclusiveDiMuonLinesConf
from Hlt2Lines.Hlt2InclusiveMuonLines   import Hlt2InclusiveMuonLinesConf
from Hlt2Lines.Hlt2InclusivePhiLines    import Hlt2InclusivePhiLinesConf
from Hlt2Lines.Hlt2TopologicalLines     import Hlt2TopologicalLinesConf
from Hlt2Lines.Hlt2XGammaLines          import Hlt2XGammaLinesConf
from Hlt2Lines.Hlt2B2HHLines            import Hlt2B2HHLinesConf
from Hlt2Lines.Hlt2B2LLXLines           import Hlt2B2LLXLinesConf
from Hlt2Lines.Hlt2DisplVerticesLines   import Hlt2DisplVerticesLinesConf
from Configurables import RichRecSysConf

class Hlt2Conf(LHCbConfigurableUser):
    __used_configurables__ = [ Hlt2B2DXLinesConf
                             , Hlt2B2JpsiXLinesConf
                             , Hlt2B2PhiXLinesConf
          		     , Hlt2Bs2JpsiPhiPrescaledAndDetachedLinesConf
                             , Hlt2Bs2JpsiPhiLinesConf
          		     , Hlt2Bd2JpsiKstarLinesConf
          		     , Hlt2Bu2JpsiKLinesConf  
	                     , Hlt2InclusiveDiMuonLinesConf
                             , Hlt2InclusiveMuonLinesConf
                             , Hlt2InclusivePhiLinesConf
                             , Hlt2TopologicalLinesConf
                             , RichRecSysConf
                             , Hlt2XGammaLinesConf
                             , Hlt2B2HHLinesConf
                             , Hlt2B2LLXLinesConf
                             , Hlt2DisplVerticesLinesConf ]
    __slots__ = {
         "DataType"                   : '2009'    # datatype is one of 2009, MC09, DC06...
       , "HltType"                    : 'Hlt1+Hlt2' # only care about Hlt2
       , "Hlt2Requires"               : 'L0+Hlt1'  # require L0 and Hlt1 pass before running Hlt2
       , "ActiveHlt2Lines"            : [] # list of lines to be added
       , "HistogrammingLevel"         : 'None' # or 'Line'
       , "ThresholdSettings"          : {} # ThresholdSettings predefined by Configuration
       , "Hlt2Tracks"                 : "Hlt/Track/Long"
         }

###################################################################################
#
# Get Hlt2 lines Configurables
#
    def getHlt2Configurables(self):
        """
        Return a dictionary of configurables
        """
        #
        # 1) convert hlt type to trigger categories
        #
        hlttype           = self.getProp("HltType")
        # Dictionary of Hlt2 type
        trans = { 'Hlt2' : 'TOPO+LEPT+PHI+EXCL'  # @todo need express as well
                }
        for short,full in trans.iteritems() : hlttype = hlttype.replace(short,full)
        #
        # 2) define what categories stand for
        # There are the strings used in HltThresholdSettings
        #
        type2conf = { 'TOPO' : [ Hlt2TopologicalLinesConf
                               , Hlt2B2DXLinesConf ]
                    , 'LEPT' : [ Hlt2InclusiveDiMuonLinesConf
                               , Hlt2InclusiveMuonLinesConf ]
                    , 'PHI'  : [ Hlt2InclusivePhiLinesConf ]
                    , 'EXCL' : [ Hlt2Bs2JpsiPhiPrescaledAndDetachedLinesConf
                               , Hlt2Bs2JpsiPhiLinesConf
                               , Hlt2Bd2JpsiKstarLinesConf
                               , Hlt2Bu2JpsiKLinesConf    
                               , Hlt2B2JpsiXLinesConf
                               , Hlt2B2PhiXLinesConf
                               , Hlt2XGammaLinesConf
                               , Hlt2B2HHLinesConf
                               , Hlt2B2LLXLinesConf
                               , Hlt2DisplVerticesLinesConf ]
                      }
        #
        # Now, decode
        #
        usedType2conf = {}
        for i in hlttype.split('+'):
            if i == '' : continue # no operation...
            if i == 'NONE' : continue # no operation...
            if i == 'Hlt1' : continue # no operation...
            if i not in type2conf : raise AttributeError, "unknown HltType fragment '%s'"%i
            usedType2conf[i] = type2conf[i]

#        print '# will use ', usedType2conf
        return usedType2conf
        
###################################################################################
#
# Threshold settings
#
# This is copied and edited from Hlt1.confType().
# One could outsource that to some function, but who cares?
#
    def hlt2Lines(self,Hlt2):
        """
        The actual lines
        """
        Hlt2.Members += [ Sequence('Hlt2Lines',ModeOR=True,ShortCircuit=False) ]
        ThresholdSettings = self.getProp("ThresholdSettings")
        #print "# Hlt2 thresholds:", ThresholdSettings
        #
        type2conf = self.getHlt2Configurables()
        #
        # Loop over thresholds
        #
        for i in type2conf :
            #print '# TYPE2CONF', i
            for confs in type2conf[i] :
                #print '# CONF', confs
                if confs not in self.__used_configurables__ : raise AttributeError, "configurable for '%s' not in list of used configurables"%i
                log.info( '# requested ' + i + ', importing ' + str(type2conf[i])  )
                # FIXME: warning: the next is 'brittle': if someone outside 
                #        does eg. HltMuonLinesConf(), it will get activated
                #        regardless of whether we do it over here...
                #        So anyone configuring some part explictly will _always_ get
                #        that part of the Hlt run, even if it does not appear in HltType...
                if ThresholdSettings:
                    from HltThresholdSettings import SetThresholds
                    SetThresholds(ThresholdSettings,confs)
                    
        #
        # Obsolete. This is now commented out.
        #
        # importOptions( "$HLTSELECTIONSROOT/options/Hlt2Lines.py" ) # I AM OBSOLETE : KILL ME!!!
###################################################################################
#
# Reco
#
    def hlt2Reconstruction(self,Hlt2):
        """
        Reconstruction
        """
        #  Full reconstruction of all tracks 
        from HltConf.HltReco import HltRecoSequence
        Hlt2.Members += [ HltRecoSequence ]

###################################################################################
#
# Particle making
#
    def hlt2Particles(self,Hlt2):
        """
        Hlt2 sequencing (was options/Hlt2Particles.py)
        """
        SeqHlt2Particles = Sequence('SeqHlt2Particles'
                                    , MeasureTime = True 
                                    , IgnoreFilterPassed = True) # do all 
        Hlt2.Members += [ SeqHlt2Particles ]
        SeqHlt2Particles.Members += [ self.hlt2Charged() ] # charged
        from HltCaloReco import hlt2Calo # calo configurable
        SeqHlt2Particles.Members += [ hlt2Calo( [ self.getProp("Hlt2Tracks") ] ) ] # list or strings
        SeqHlt2Particles.Members += [ self.hlt2Muon() ] # muon
        self.hlt2Protos(SeqHlt2Particles)          # protos
        
###################################################################################
#
# ProtoParticles
#
    def hlt2Protos(self,SeqHlt2Particles):
        """
        protoparticles 
        """
        from Configurables import ChargedProtoPAlg, ChargedProtoCombineDLLsAlg, NeutralProtoPAlg, TrackSelector
        Hlt2ChargedProtoPAlg = ChargedProtoPAlg('Hlt2ChargedProtoPAlg')
        Hlt2ChargedProtoPAlg.InputTrackLocation = self.getProp("Hlt2Tracks")
        Hlt2ChargedProtoPAlg.OutputProtoParticleLocation = "Hlt/ProtoP/Charged"
        Hlt2ChargedProtoPAlg.addTool( TrackSelector, name="TrackSelector")
        Hlt2ChargedProtoPAlg.TrackSelector.AcceptClones = False
        Hlt2ChargedProtoPAlg.InputMuonPIDLocation = "Hlt/Muon/MuonPID"
        ## Calo PID
        Hlt2ChargedProtoPAlg.UseCaloSpdPID = True 
        Hlt2ChargedProtoPAlg.UseCaloPrsPID = True 
        Hlt2ChargedProtoPAlg.UseCaloEcalPID = True 
        Hlt2ChargedProtoPAlg.UseCaloHcalPID = True 
        Hlt2ChargedProtoPAlg.UseCaloBremPID = True 
        ##Hlt2ChargedProtoPAlg.UseRichPID = False  // Protos will NOT have any RICH information - HltRichPIDsKaons will not work
        Hlt2ChargedProtoPAlg.UseRichPID = True     ## Use this to add RICH info to the HLT protos, needed for HltRichPIDsKaons
        Hlt2ChargedProtoPAlg.UseMuonPID = True 
        Hlt2ChargedProtoPAlg.UseVeloPID = False 
        SeqHlt2Particles.Members += [ Hlt2ChargedProtoPAlg ]

        Hlt2ChargedProtoCombDLL = ChargedProtoCombineDLLsAlg('Hlt2ChargedProtoCombDLL')
        Hlt2ChargedProtoCombDLL.ProtoParticleLocation = "Hlt/ProtoP/Charged" 
        SeqHlt2Particles.Members += [ Hlt2ChargedProtoCombDLL ]

        HltNeutralProtoPAlg = NeutralProtoPAlg('HltNeutralProtoPAlg')
        # Overwrite some default offline settings with HLT special settings (taken from CaloReco.opts)
        HltNeutralProtoPAlg.PhotonIDName = "HltPhotonPID"
        from Configurables import CaloPhotonEstimatorTool
        ToolSvc().addTool(CaloPhotonEstimatorTool, name="HltPhotonPID")
        ToolSvc().HltPhotonPID.TableLocation = "Hlt/Calo/ClusterMatch"
        importOptions( "$CALOPIDSROOT/options/HltPhotonPDF.opts" )
        
        SeqHlt2Particles.Members += [ HltNeutralProtoPAlg ]

        
###################################################################################
#
# MuonID
#
    def hlt2Muon(self):
        """
        Muon ID options
        """
        from MuonID import ConfiguredMuonIDs
        from Configurables import MuonRec, MuonIDAlg
        cm = ConfiguredMuonIDs.ConfiguredMuonIDs(data=self.getProp("DataType"))
        HltMuonIDAlg = cm.configureMuonIDAlg("HltMuonIDAlg")
        HltMuonIDAlg.TrackLocation = "Hlt/Track/Long"
        HltMuonIDAlg.MuonIDLocation = "Hlt/Muon/MuonPID"
        HltMuonIDAlg.MuonTrackLocation = "Hlt/Track/Muon"
        HltMuonIDSeq = GaudiSequencer("HltMuonIDSeq")
        HltMuonIDSeq.Members += ["MuonRec", HltMuonIDAlg]

        return HltMuonIDSeq


###################################################################################
#
# Charged Particle making
#
    def hlt2Charged(self):
        """
        Charged particles
        """
        from Configurables import NumberOfTracksFilter
        from Configurables import HltInsertTrackErrParam
        SeqHlt2Charged = Sequence('SeqHlt2Charged'
                                        , MeasureTime = True
                                        , IgnoreFilterPassed = False)        
        #
        # @todo TEMPORARY kill huge events
        #
        NumberOfTracksFilter =  NumberOfTracksFilter()
        NumberOfTracksFilter.TrackLocations = [ self.getProp("Hlt2Tracks") ]
        NumberOfTracksFilter.MaxTracks = 1000 
        SeqHlt2Charged.Members += [ NumberOfTracksFilter ]
        #
        # MC truth associated tracks
        #
        SeqHlt2Charged.Members += [ Sequence('SeqTrueSignalTracks') ] #  debug
        #
        # Hacking of errors
        #
        HltInsertTrackErrParam = HltInsertTrackErrParam()
        HltInsertTrackErrParam.InputLocation = "Hlt/Track/Long" 
        SeqHlt2Charged.Members += [ HltInsertTrackErrParam ]
        return SeqHlt2Charged
       
###################################################################################
#
# Requirements
#
    def hlt2Requirements(self,Hlt2):
        """
        Requirements
        """
        Hlt2.Members += [ Sequence("Hlt2Requirements") ]
        Sequence("Hlt2Requirements").Members = [ ]
        reqs = self.getProp('Hlt2Requires')
        if reqs.upper() != "NONE" :
            from Configurables import LoKi__HDRFilter   as HltFilter
            from Configurables import LoKi__L0Filter    as L0Filter
            hlt2requires = { 'L0'   : L0Filter( 'L0Pass',          Code = "L0_DECISION" )
                             , 'Hlt1' : HltFilter('Hlt1GlobalPass' , Code = "HLT_PASS('Hlt1Global')" )
                             }
            for i in reqs.split('+') :
                if i : Sequence("Hlt2Requirements").Members.append( hlt2requires[i] )

###################################################################################
#
# Main configuration
#
    def __apply_configuration__(self):
        """
        Hlt2 configuration
        """
        Hlt2 = Sequence("Hlt2", Context = 'HLT' )
        if Hlt2 not in Sequence("Hlt").Members : Sequence("Hlt").Members += [ Hlt2 ]
        # requirements
        self.hlt2Requirements(Hlt2)        
        # reco
        self.hlt2Reconstruction(Hlt2)
        importOptions( "$HLTCONFROOT/options/HltTrackAssociator.py" )
        # reco
        self.hlt2Particles(Hlt2)
        # lines
        self.hlt2Lines(Hlt2)
        
