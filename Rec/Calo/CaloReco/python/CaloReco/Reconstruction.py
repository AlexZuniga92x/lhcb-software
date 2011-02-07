#!/usr/bin/env python
# =============================================================================
# $Id: Reconstruction.py,v 1.12 2010-05-20 09:47:06 odescham Exp $
# =============================================================================
## The major building blocks of Calorimeter Reconstruction
#  @author Vanya BELYAEV Ivan.Belyaev@nikhe.nl
#  @date 2008-07-17
# =============================================================================
"""
The major building blocks of Calorimeter Reconstruction
"""
# =============================================================================
__author__  = "Vanya BELYAEV Ivan.Belyaev@nikhef.nl"
__version__ = "CVS tag $Name: not supported by cvs2svn $, version $Revision: 1.12 $"
# =============================================================================
__all__ = (
    'clustersReco'   , 
    'photonsReco'    , 
    'mergedPi0sReco' 
    )
# =============================================================================
from Gaudi.Configuration import *

from Configurables import GaudiSequencer

from GaudiKernel.SystemOfUnits import MeV, GeV


from CaloKernel.ConfUtils import ( prntCmp        ,
                                   addAlgs        ,
                                   hltContext     ,
                                   setTheProperty ,
                                   onDemand       ,
                                   caloOnDemand   ,
                                   getAlgo        ) 

import logging
_log = logging.getLogger('CaloReco')

# =============================================================================
## prepare the digits for the recontruction
def digitsReco  ( context            ,
                  enableRecoOnDemand ,
                  createADC          ) :
    """
    Prepare the digits for the recontruction
    """

    from CaloDAQ.CaloDigits  import caloDigits

    _log.warning('CaloReco.digitsReco is deprecated, use CaloDigitConf instead')

    # warning : the caloDigits TES should be context-independent
    return caloDigits ( context            ,
                        enableRecoOnDemand ,
                        createADC          )

## ============================================================================
## define the recontruction of Ecal clusters
def clusterReco ( context , enableRecoOnDemand ) :
    """
    Define the recontruction of Ecal Clusters
    """
    
    from Configurables import ( CaloDigitFilterAlg       ,
                                CellularAutomatonAlg     ,
                                CaloSharedCellAlg        ,
                                CaloClusterCovarianceAlg ,
                                CaloClusterizationTool) 

    # cluster TES  is 'almost' context-independent : single HLT TES whatever the HLT-type else 'offline' TES
    ## Warning MUST be synchronous with CaloAlgUtils TES settings
    _cont = context
    if '' != context and 'OFFLINE' != _cont.upper() and _cont.upper().find( 'HLT' ) != -1 :
        context = 'Hlt'
    else :
        context = ''
    

    ## Define the context-dependent sequencer
    seq   = getAlgo ( GaudiSequencer           , "ClusterReco", context , "Rec/Calo/EcalClusters" , enableRecoOnDemand )
    filter= getAlgo ( CaloDigitFilterAlg       , "CaloDigitFilter",context)
    clust = getAlgo ( CellularAutomatonAlg     , "EcalClust"  , context )
    share = getAlgo ( CaloSharedCellAlg        , "EcalShare"  , context )  
    covar = getAlgo ( CaloClusterCovarianceAlg , "EcalCovar"  , context ) 
    seq.Members = [ filter, clust , share , covar ]
    setTheProperty ( seq , 'Context' , context )



    ## setup onDemand for SplitClusters
    if enableRecoOnDemand : 
        splitSeq   = mergedPi0Reco( context , enableRecoOnDemand , True, False, False)
        onDemand ( 'Rec/Calo/EcalSplitClusters' , splitSeq , context ) 
        _log.info    ('CaloReco/ClusterReco: creation-on-demand of clusters is enabled')


    ## printout
    _log.debug ( 'Configure Ecal Cluster Reco Seq : %s   for %s : ' % (seq.name()  , context) )
    
    ##    
    return seq

# ============================================================================
## define the recontruction of  Single Photons
def photonReco ( context , enableRecoOnDemand, useTracks = True , useSpd = False, usePrs = False , trackLocations = [], neutralID = True) :
    """
    Define the recontruction of Single Photon Hypo
    """
    
    from Configurables import   CaloSinglePhotonAlg                
    from Configurables import   CaloHypoAlg     
    from Configurables import ( CaloSelectCluster                  , 
                                CaloSelectClusterWithPrs           ,
                                CaloSelectNeutralClusterWithTracks , 
                                CaloSelectNeutralClusterWithSpd    ,
                                CaloSelectChargedClusterWithSpd    , 
                                CaloSelectorNOT                    ,
                                CaloSelectorOR                     )
    
    from Configurables import ( CaloExtraDigits ,
                                CaloECorrection , 
                                CaloSCorrection , 
                                CaloLCorrection ,
                                CaloCorrectionBase) 
    
    
    ## build the context-dependent sequence (  TrackMatch + SinglePhotonRec )
    seq = getAlgo ( GaudiSequencer , "PhotonReco" , context , "Rec/Calo/Photons"  , enableRecoOnDemand   )

    # 1/ PhotonMatch from CaloPIDs (if tracking is requested)
    if useTracks : 
        from CaloPIDs.PIDs import trackMatch
        tm =  trackMatch ( context,enableRecoOnDemand, trackLocations )
        addAlgs ( seq , tm ) 
    

    ##2/ SinglePhotonRec alg
    alg = getAlgo ( CaloSinglePhotonAlg, "SinglePhotonRec" , context )
    alg.PropertiesPrint = False
    
    # cluster selection tools:
    ### a/ generic selection (energy/multiplicity)
    alg.addTool ( CaloSelectCluster  , "PhotonCluster" )
    alg.SelectionTools = [ alg.PhotonCluster ]
    alg.PhotonCluster.MinEnergy = 150.*MeV
    
    ### b/ Neutral cluster (track-based and/or Spd/Prs-based)    
    if   useTracks     :
        alg.addTool ( CaloSelectNeutralClusterWithTracks , "NeutralCluster" )
        alg.NeutralCluster.MinChi2 = 4
        alg.SelectionTools += [ alg.NeutralCluster ]
        _log.info    ('CaloReco/PhotonReco: Configure Neutral Cluster Selector with Tracks     : %s' %  alg.NeutralCluster.getFullName() )

    if usePrs :
        alg.addTool ( CaloSelectClusterWithPrs           , "ClusterWithPrs" )
        alg.ClusterWithPrs.MinEnergy = 10.*MeV
        alg.SelectionTools += [ alg.ClusterWithPrs ]
        _log.warning ('CaloReco/PhotonReco: Configure Cluster Selector with  Prs       : %s' %  alg.ClusterWithPrs.getFullName() )

    if  useSpd  :
        alg.addTool ( CaloSelectNeutralClusterWithSpd    , "NeutralClusterWithSpd"    )        
        alg.SelectionTools += [ alg.NeutralClusterWithSpd ]
        _log.warning ('CaloReco/PhotonReco: Configure Neutral Cluster Selector with !Spd : %s as %s ' %   alg.NeutralClusterWithSpd.getFullName()  )
        
        
    ## hypo tools : add Spd/Prs digits
    alg.addTool ( CaloExtraDigits , 'SpdPrsExtraG' )
    alg.HypoTools = [ alg.SpdPrsExtraG ]
    
    ## correction tools : E/S/L
    alg.addTool ( CaloECorrection , 'ECorrection' )
    alg.addTool ( CaloSCorrection , 'SCorrection' )
    alg.addTool ( CaloLCorrection , 'LCorrection' )
    
    ecorr = alg.ECorrection
    scorr = alg.SCorrection
    lcorr = alg.LCorrection
    
    alg.CorrectionTools2 = [ ecorr ,
                             scorr ,
                             lcorr
                             ]


    # temporary : will be from condDB
    from Corrections import eCorrection, sCorrection, lCorrection
    ecorr = eCorrection ( ecorr )
    scorr = sCorrection ( scorr )
    lcorr = lCorrection ( lcorr )
                      

    # update the sequence
    addAlgs ( seq , alg ) 

    ## 3/ PhotonID
    from CaloPIDs.PIDs import PhotonID
    if neutralID : 
        pID =  PhotonID ( context,enableRecoOnDemand, useTracks )
        addAlgs ( seq , pID ) 


    if enableRecoOnDemand : 
        _log.info    ('CaloReco/PhotonReco: creation-on-demand of photons  is enabled')


    ## global context
    setTheProperty ( seq , 'Context' , context )

    ## printout
    _log.debug ( 'Configure Photon Reco Seq : %s  for : %s' % (seq.name() , context ) )
        
    return seq

# ============================================================================
## define the recontruction of Electorn Hypos
def electronReco ( context , enableRecoOnDemand , useTracksE = True , useSpdE = True, usePrsE = True, trackLocations = [] ) :
    """
    Define the reconstruction of
    """

    from Configurables import   CaloElectronAlg                
    from Configurables import   CaloHypoAlg 
    
    from Configurables import ( CaloSelectCluster                  , 
                                CaloSelectClusterWithPrs           ,
                                CaloSelectNeutralClusterWithTracks , 
                                CaloSelectNeutralClusterWithSpd    ,
                                CaloSelectChargedClusterWithSpd    , 
                                CaloSelectorNOT                    )
    
    from Configurables import ( CaloExtraDigits ,
                                CaloECorrection , 
                                CaloSCorrection , 
                                CaloLCorrection ) 
    

    ## build the context-dependent sequence (  TrackMatch + SingleElectronRec )
    seq = getAlgo ( GaudiSequencer , "ElectronReco" , context  , 'Rec/Calo/Electrons' , enableRecoOnDemand   ) 

    # 1/ ElectronMatch from CaloPIDs (if useTracks)
    if useTracksE :
        from CaloPIDs.PIDs import trackMatch
        tm =  trackMatch ( context,enableRecoOnDemand, trackLocations )
        addAlgs ( seq , tm ) 

    ## 2/ Electron Rec alg
    alg = getAlgo ( CaloElectronAlg ,  'SingleElectronRec', context  ) 
    alg.PropertiesPrint = False

    # cluster selection tools:

    ## 1/ generic selection (energy/multiplicity)
    alg.addTool ( CaloSelectCluster               , "ElectronCluster" )
    alg.SelectionTools = [ alg.ElectronCluster ]
    alg.ElectronCluster.MinEnergy = 150.*MeV

    ## 2/  hits in Spd
    if useSpdE : 
        alg.addTool ( CaloSelectChargedClusterWithSpd , "ChargedClusterWithSpd"  )
        alg.SelectionTools += [ alg.ChargedClusterWithSpd ]
        _log.info    ('CaloReco/ElectronReco: Configure Charged Cluster Selector with Spd     : %s' %  alg.ChargedClusterWithSpd.getFullName() )

    ## 3/ energy in Prs
    if usePrsE :  
        alg.addTool ( CaloSelectClusterWithPrs        , "ClusterWithPrs"  )
        alg.ClusterWithPrs.MinEnergy = 10 * MeV
        alg.SelectionTools += [ alg.ClusterWithPrs ]
        _log.info    ('CaloReco/ElectronReco: Configure Cluster Selector with Prs    : %s' %  alg.ClusterWithPrs.getFullName() )

    ## 4/  match with tracks
    if useTracksE :
        alg.addTool ( CaloSelectorNOT   , "ChargedCluster"  )
        clnot = alg.ChargedCluster
        clnot.addTool ( CaloSelectNeutralClusterWithTracks , "NotNeutralCluster" )
        clnot.NotNeutralCluster.MinChi2 = 25
        clnot.SelectorTools = [ clnot.NotNeutralCluster ]
        alg.SelectionTools += [ alg.ChargedCluster ]
        _log.info    ('CaloReco/ElectronReco: Configure Charged Cluster Selector with Tracks     : %s' %  alg.ChargedCluster.getFullName() )


    ## hypo tools : add Spd/Prs digits
    alg.addTool ( CaloExtraDigits , 'SpdPrsExtraE' )
    alg.HypoTools = [ alg.SpdPrsExtraE ]

    ## correction tools : E/S/L
    alg.addTool ( CaloECorrection , 'ECorrection' )
    alg.addTool ( CaloSCorrection , 'SCorrection' )
    alg.addTool ( CaloLCorrection , 'LCorrection' )

    ecorr = alg.ECorrection
    scorr = alg.SCorrection
    lcorr = alg.LCorrection

    alg.CorrectionTools2 = [ ecorr , scorr , lcorr ]

    # temporary : will be from condDB
    from CaloReco.Corrections import eCorrection, sCorrection, lCorrection
    ecorr = eCorrection ( ecorr )
    scorr = sCorrection ( scorr )
    lcorr = lCorrection ( lcorr )

    
    ## update the sequence
    addAlgs ( seq , alg ) 

    # 
    if enableRecoOnDemand : 
        _log.info    ('CaloReco/ElectronReco: creation-on-demand of electrons is enabled')

    # global context 
    setTheProperty ( seq , 'Context' , context )

    ## printout
    _log.debug ( 'Configure Electron Reco Seq : %s  for : %s' % (seq.name() , context ))

    return seq

    
# =============================================================================
## define the reconstruction of Merged Pi0s Hypos 
def mergedPi0Reco ( context , enableRecoOnDemand , clusterOnly = False , neutralID = True , useTracks = True) :
    """
    Define the recontruction of Merged Pi0s
    """

    from Configurables import   CaloMergedPi0Alg 
    from Configurables import   CaloHypoAlg 

    from Configurables import ( CaloExtraDigits ,
                                CaloECorrection , 
                                CaloSCorrection , 
                                CaloLCorrection ,
                                CaloCorrectionBase) 


    # build the sequences
    seq = getAlgo ( GaudiSequencer , 'MergedPi0Reco' , context )
    sseq = getAlgo ( GaudiSequencer , 'SplitClusterReco' , context )
        
    ## Merged Pi0
    if clusterOnly :
        pi0 = getAlgo ( CaloMergedPi0Alg , 'SplitClustersRec', context )
        pi0.CreateSplitClustersOnly = True
        addAlgs ( sseq , pi0 ) 
    else :
        pi0 = getAlgo ( CaloMergedPi0Alg , 'MergedPi0Rec', context )        


    pi0.PropertiesPrint = False
    
    pi0.EtCut    = 2. * GeV;
    # default setting (possibly updated from condDB)
    pi0.addTool( CaloCorrectionBase, 'ShowerProfile')
    pi0.addTool( CaloCorrectionBase, 'Pi0SCorrection')
    pi0.addTool( CaloCorrectionBase, 'Pi0LCorrection')
    shower = pi0.ShowerProfile
    pSCorr = pi0.Pi0SCorrection
    pLCorr = pi0.Pi0LCorrection
    from Corrections import eCorrection, sCorrection, lCorrection,showerProfile
    shower  = showerProfile( shower )
    pSCorr  = sCorrection( pSCorr  )
    pLCorr  = lCorrection( pLCorr  )    

    if clusterOnly :
        setTheProperty ( sseq , 'Context' , context )
        return sseq

    ## MergedPi0 sequence

    ## 2/ SplitPhotons
#    splitg = getAlgo ( CaloHypoAlg , 'PhotonFromMergedRec' , context )    
#    splitg.HypoType = "SplitPhotons";
#    splitg.PropertiesPrint = False   
    
    ## Add Prs/Spd digits
    pi0.addTool ( CaloExtraDigits , 'SpdPrsExtraS' )
    pi0.addTool ( CaloExtraDigits , 'SpdPrsExtraM' )
    ## correction tools
    pi0.addTool ( CaloECorrection , 'ECorrection' )
    pi0.addTool ( CaloSCorrection , 'SCorrection' )
    pi0.addTool ( CaloLCorrection , 'LCorrection' )
    ecorr = pi0.ECorrection
    scorr = pi0.SCorrection
    lcorr = pi0.LCorrection
    
    ## temporary : will be from condDB
    ecorr = eCorrection ( ecorr )
    scorr = sCorrection ( scorr )
    lcorr = lCorrection ( lcorr )
    pi0.Tools = [ pi0.SpdPrsExtraS , ecorr , scorr , lcorr ]
    pi0.Pi0Tools = [ pi0.SpdPrsExtraM ]
    addAlgs ( seq , pi0 ) 
#    addAlgs ( seq , splitg ) 
    
    ## 3/ (PhotonFrom)MergedID
    if neutralID : 
        from CaloPIDs.PIDs import (MergedID , PhotonFromMergedID )
        idSeq = getAlgo( GaudiSequencer, "MergedIDSeq",context)
        mID =  MergedID ( context,enableRecoOnDemand, useTracks )
        pmID =  PhotonFromMergedID( context, enableRecoOnDemand, useTracks )
        idSeq.Members = [ mID, pmID ]  
        #idSeq.Members = [ mID ]  
        addAlgs ( seq , idSeq ) 

    # propagate the context
    setTheProperty ( seq , 'Context' , context )


    ## onDemand
    if enableRecoOnDemand :
            onDemand ( 'Rec/Calo/SplitPhotons'      , seq , context ) 
            onDemand ( 'Rec/Calo/MergedPi0s'        , seq , context )
            ## onDemand ( 'Rec/Calo/EcalSplitClusters' , seq , context )  ## ??

    ##
    if enableRecoOnDemand : 
        _log.info    ('CaloReco/MergedPi0 Reco: creation-on-demand of mergedPi0/SplitPhotons  is enabled')


    ## printout
    _log.debug ( 'Configure MergedPi0 Reco Seq : %s  for : %s' %( seq.name() , context ))

    return seq


# =============================================================================
if '__main__' == __name__ :
    print __doc__
