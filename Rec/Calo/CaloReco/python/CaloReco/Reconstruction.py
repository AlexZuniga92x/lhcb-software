#!/usr/bin/env python
# =============================================================================
# $Id: Reconstruction.py,v 1.9 2010-03-08 01:19:39 odescham Exp $
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
__version__ = "CVS tag $Name: not supported by cvs2svn $, version $Revision: 1.9 $"
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
    
    from Configurables import ( CellularAutomatonAlg     ,
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
    clust = getAlgo ( CellularAutomatonAlg     , "EcalClust"  , context )
    share = getAlgo ( CaloSharedCellAlg        , "EcalShare"  , context ) 
    covar = getAlgo ( CaloClusterCovarianceAlg , "EcalCovar"  , context )
    seq.Members = [ clust , share , covar ]
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
                                CaloLCorrection ) 
    
    
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
                                CaloLCorrection ) 


    # build the sequence
    seq = getAlgo ( GaudiSequencer , 'MergedPi0Reco' , context )
        
    ## Merged Pi0
    if clusterOnly :
        pi0 = getAlgo ( CaloMergedPi0Alg , 'SplitClustersRec', context )
        pi0.CreateSplitClustersOnly = True
    else :
        pi0 = getAlgo ( CaloMergedPi0Alg , 'MergedPi0Rec', context )        




    # temporary : will be from condDB
    pi0.PropertiesPrint = False
    pi0.TrShOut_nospd = [ -0.0060,  2.4956,115.0827,  9.8842,  0.0320,  2.0982,  1.0302,  0.0409,  0.0030, -9.6135 ]
    pi0.TrShMid_nospd = [  0.0464,  2.0384, 36.5885,  8.0260,  0.0460,  2.3936,  1.0703,  0.1611,  0.0238, -5.8899 ]
    pi0.TrShInn_nospd = [  0.0981,  2.2529, 33.8837,  8.0532,  0.0654,  2.2046,  1.1092,  0.1645,  0.0248, -5.7248 ]
    
    pi0.TrShOut_spd   = [  0.0486,  2.7847, 68.4815,  9.0870,  0.0116,  1.2591,  1.0464, -0.0900,  0.0005,-12.9098 ]
    pi0.TrShMid_spd   = [  0.0882,  1.7910, 24.4324,  7.4802,  0.0258,  2.7719,  1.1294, -0.0802,  0.0024, -9.7273 ]
    pi0.TrShInn_spd   = [  0.1111,  1.7909, 18.0852,  7.1122,  0.0261,  1.3889,  1.1846, -0.0934,  0.0029, -8.9966 ]
    
    pi0.SPar     = [ 0.102 , 0.129 , 0.144  ]
    pi0.LPar_Al1 = [ 17.936       ,  19.053       , 22.305 ] 
    pi0.LPar_Al2 = [   1.15  ,  1.53  ,  2.04  ]
    pi0.LPar_Al3 = [ 0.0392 , 0.455  , 0.0281  ]
    pi0.LPar_Be1 = [ 54.54  , 60.87  , 58.04   ]
    pi0.LPar_Be2 = [ 4.02  ,  4.15  ,  4.38    ]
    pi0.LPar_Be3 = [  0.0308 , 0.0597 , 0.0247 ]
    pi0.LPar_Be0 = [   0.,0.,0. ] ## //unused
    pi0.LPar_z0  = [ 12566. ]
    
    # temporary : will be from condDB
    pi0.EtCut    = 2. * GeV;


    addAlgs ( seq , pi0 ) 

    if clusterOnly :
        setTheProperty ( seq , 'Context' , context )
        return seq


    ## 2/ SplitPhotons
    splitg = getAlgo ( CaloHypoAlg , 'PhotonFromMergedRec' , context )    
    splitg.HypoType = "SplitPhotons";
    splitg.PropertiesPrint = False   
    
    ## Add Prs/Spd digits
    splitg.addTool ( CaloExtraDigits , 'SpdExtraG' )
    ## correction tools
    splitg.addTool ( CaloECorrection , 'ECorrection' )
    splitg.addTool ( CaloSCorrection , 'SCorrection' )
    splitg.addTool ( CaloLCorrection , 'LCorrection' )
    ecorr = splitg.ECorrection
    scorr = splitg.SCorrection
    lcorr = splitg.LCorrection
    
    ## temporary : will be from condDB
    from Corrections import eCorrection, sCorrection, lCorrection
    ecorr = eCorrection ( ecorr )
    scorr = sCorrection ( scorr )
    lcorr = lCorrection ( lcorr )
    splitg.Tools = [ splitg.SpdExtraG , ecorr , scorr , lcorr ]
    addAlgs ( seq , splitg ) 
    
    ## 3/ (PhotonFrom)MergedID
    if neutralID : 
        from CaloPIDs.PIDs import (MergedID , PhotonFromMergedID )
        idSeq = getAlgo( GaudiSequencer, "MergedIDSeq",context)
        mID =  MergedID ( context,enableRecoOnDemand, useTracks )
        pmID =  PhotonFromMergedID( context, enableRecoOnDemand, useTracks )
        idSeq.Members = [ mID, pmID ]  
        #idSeq.Members = [ mID ]  
        addAlgs ( seq , mID ) 

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
