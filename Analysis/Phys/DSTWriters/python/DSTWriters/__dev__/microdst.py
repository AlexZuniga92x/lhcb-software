"""

"""

__author__ = "Juan Palacios <juan.palacios@nikhef.nl>"

__all__ = ('microDSTElements',
           'microDSTStreamConf',
           'stripMicroDSTElements',
           'stripMicroDSTStreamConf')

from GaudiConf.Configuration import *
from Configurables import OutputStream

from streamconf import OutputStreamConf

from microdstelements import  (CloneRecHeader,
                               CloneRecSummary,
                               CloneODIN,
                               GlobalEventCounters,
                               ClonePVs,
                               CloneParticleTrees,
                               ClonePVRelations,
                               CloneBackCat,
                               CloneBTaggingInfo,
                               ReFitAndClonePVs,
                               CloneLHCbIDs,
                               CloneRawBanks)

from GaudiConfUtils.ConfigurableGenerators import LoKi__CounterAlg as CounterAlg
_gecConfig = CounterAlg(
    Preambulo = [
    "from LoKiTracks.decorators  import *" ,
    "from LoKiCore.functions     import *" ,
    ] ,
    ##
    Variables = {
    #
    ## Reco information
    #
    "nPV"           : "CONTAINS ( 'Rec/Vertex/Primary'            ) " ,
    "nBest"         : "CONTAINS ( 'Rec/Track/Best'                ) " ,
    "nVelo"         : "TrNUM    ( 'Rec/Track/Best' , TrVELO       ) " ,
    "nLong"         : "TrNUM    ( 'Rec/Track/Best' , TrLONG       ) " ,
    "nUpstream"     : "TrNUM    ( 'Rec/Track/Best' , TrUPSTREAM   ) " ,
    "nDownstream"   : "TrNUM    ( 'Rec/Track/Best' , TrDOWNSTREAM ) " ,    
    "nTTrack"       : "TrNUM    ( 'Rec/Track/Best' , TrTTRACK     ) " ,
    "nBack"         : "TrNUM    ( 'Rec/Track/Best' , TrBACKWARD   ) " ,
    "nMuon"         : "CONTAINS ( 'Rec/Track/Muon'                ) " ,
    "nEcalClusters" : "CONTAINS ( 'Rec/Calo/EcalClusters'         ) " ,
    #
    ## some ``raw'' information
    #
    "nSpd"          : "recSummary ( LHCb.RecSummary.nSPDhits      , 'Raw/Spd/Digits'    ) " ,
    "nOTClusters"   : "recSummary ( LHCb.RecSummary.nOTClusters   , 'Raw/OT/Times'      ) " ,
    "nITClusters"   : "recSummary ( LHCb.RecSummary.nITClusters   , 'Raw/IT/Clusters'   ) " ,
    "nTTClusters"   : "recSummary ( LHCb.RecSummary.nITClusters   , 'Raw/TT/Clusters'   ) " ,
    "nVeloClusters" : "recSummary ( LHCb.RecSummary.nVeloClusters , 'Raw/Velo/Clusters' ) " 
    }
    )

def microDSTElements() :
    return [CloneRecHeader(),
            CloneRecSummary(),
            CloneODIN(),
            GlobalEventCounters(configGenerator=_gecConfig),
            ClonePVs(),
            CloneParticleTrees(copyProtoParticles = True),
            ClonePVRelations("Particle2VertexRelations",True),
            CloneRawBanks( banks = [ 'ODIN'] ) ]

def microDSTStreamConf() :
    return OutputStreamConf(streamType = OutputStream,
                            fileExtension = '.mdst',
                            extraItems = ['/Event/Rec/Header#1'])

def stripMicroDSTElements() :
    return [CloneRecHeader(),
            CloneRecSummary(),
            CloneODIN(),
            GlobalEventCounters(configGenerator=_gecConfig),
            ClonePVs(),
            CloneParticleTrees(copyProtoParticles = True),
            ClonePVRelations("Particle2VertexRelations", True),
            CloneLHCbIDs(fullDecayTree = True),
            ReFitAndClonePVs(),
            CloneRawBanks( banks = [ 'ODIN',
                                     'HltSelReports' ,
                                     'HltDecReports',
                                     'L0Calo',
                                     'L0CaloFull',
                                     'L0DU',
                                     'L0Muon',
                                     'L0MuonProcCand',
                                     'L0PU'           ] ) ]

def stripMicroDSTStreamConf() :
    return OutputStreamConf(streamType = OutputStream,
                            fileExtension = '.mdst',
                            extraItems = ['/Event/Rec/Header#1',
                                          '/Event/Rec/Status#1',
                                          '/Event/Strip/Phys/DecReports#1' ])
