#!/usr/bin/env python
# =============================================================================
# $Id: Monitor.py,v 1.1 2009-08-05 17:40:24 ibelyaev Exp $
# =============================================================================
## The major building blocks of Calorimeter Monitoring
#  @author Vanya BELYAEV Ivan.Belyaev@nikhe.nl
#  @date 2008-07-17
# =============================================================================
"""
The major building blocks of Calorimeter Monitoring
"""
# =============================================================================
__author__  = "Vanya BELYAEV Ivan.Belyaev@nikhef.nl"
__version__ = "CVS tag $Name: not supported by cvs2svn $, version $Revision: 1.1 $"
# =============================================================================
__all__ = (
    'digitsMoni'     , 
    'eflowMoni'      , 
    'clustersMoni'   , 
    'hyposMoni'      , 
    'pi0Moni'        , 
    'pidsMoni'      
    )
# =============================================================================
from Gaudi.Configuration  import *

from Configurables        import   GaudiSequencer
from Configurables        import ( CaloDigitMonitor        ,
                                   CaloClusterMonitor      , 
                                   CaloHypoMonitor         ,
                                   CaloPi0Monitor          ,
                                   CaloClusterMatchMonitor ,
                                   CaloHypoMatchMonitor    ,
                                   CaloEMuMonitor          ,
                                   SpdMonitor              ,
                                   CaloEFlowAlg            )

from CaloKernel.ConfUtils import ( getAlgo        ,
                                   setTheProperty ) 

from GaudiKernel.SystemOfUnits import  ( MeV , GeV )


# =============================================================================
## prepare digit monitoring
def digitsMoni ( context ) :
    """
    
    Prepare digit monitoring
    
    """
    alg = getAlgo ( GaudiSequencer   , 
                    "CaloDigitsMoni" ,
                    context          )
    
    alg1 = getAlgo ( CaloDigitMonitor , "EcalDigitMon"  , context ) 
    alg2 = getAlgo ( CaloDigitMonitor , "HcalDigitMon"  , context ) 
    alg3 = getAlgo ( CaloDigitMonitor , "PrsDigitMon"   , context ) 
    alg4 = getAlgo ( CaloDigitMonitor , "SpdDigitMon"   , context ) 
    
    alg1.histoList   = [ "1", "2" , "3" , "6" , "7" ]
    alg2.histoList   = [ "1", "2" , "3" , "6" , "7" ]
    alg3.histoList   = [ "1", "2" , "3" , "6" , "7" ]
    alg4.histoList   = [ "1", "2" , "3" , "6" ]
    
    alg.Members = [
        alg1 ,
        alg2 ,
        alg3 ,
        alg4 , 
        ##
        getAlgo ( SpdMonitor , "SpdMon"   , context )
        ]
    
    setTheProperty ( alg , 'Context' , context )
    
    return alg


# ============================================================================
## Define Energy flow monitoring 
def eflowMoni ( context ) :
    """
    Define Energy flow omnitoring 
    """

    
    seq = getAlgo ( GaudiSequencer  , 
                    'CaloEFlowMoni' ,
                    context         )
    
    alg = getAlgo ( CaloEFlowAlg    ,
                    'EcalEFlowMon'  ,
                    context         )

    alg.histoList       = [ 'all' ]
    alg.SplitAreas      = False
    alg.OneDimension    =  True
    alg.EtFilterMax     =  1 * GeV
    alg.EnergyFilterMax = 10 * GeV
    
    seq.Members = [ alg ] 

    setTheProperty ( alg , 'Context' , context )
    
    return seq

    
# =============================================================================
## prepare cluster monitoring
def clustersMoni ( context ) :
    """    
    Prepare clusters monitoring
    
    """
    alg = getAlgo ( GaudiSequencer     , 
                    "CaloClustersMoni" ,
                    context            )
    
    alg1 = getAlgo ( CaloClusterMonitor ,
                     'EcalClusterMon'   ,
                     context            )
    
    alg2 = getAlgo ( CaloClusterMonitor      ,
                     'EcalSplitClusterMon'  ,
                     context                 )
    
    alg1.histoList = [ "1", "2" , "3" , "4" , "7" , "8" ,  "9" ]
    
    alg1.Input = 'Rec/Calo/EcalClusters'
    alg2.Input = 'Rec/Calo/EcalSplitClusters'

    alg.Members = [ alg1 , alg2 ]
    
    setTheProperty ( alg , 'Context' , context )
    
    return alg


# =============================================================================
## prepare cluster monitoring
def hyposMoni ( context ) :
    """    
    Prepare hypo monitoring
    
    """
    alg = getAlgo ( GaudiSequencer    , 
                    "CaloHyposMoni"   ,
                    context           )
    
    alg1 = getAlgo ( CaloHypoMonitor  ,
                     'ElectronMon'    ,
                     context          )
    alg2 = getAlgo ( CaloHypoMonitor  ,
                     'PhotonMon'      ,
                     context          )
    alg3 = getAlgo ( CaloHypoMonitor  ,
                     'SplitPhotonMon' ,
                     context          )
    alg4 = getAlgo ( CaloHypoMonitor  ,
                     'MergedPi0Mon'   ,
                     context          )


    alg1.Input = 'Rec/Calo/Electrons'
    alg2.Input = 'Rec/Calo/Photons'
    alg3.Input = 'Rec/Calo/SplitPhotons'
    alg4.Input = 'Rec/Calo/MergedPi0s'


    alg1 .histoList = [ "1", "2" , "3" , "7" , "8" , "9" , "10" , "11" ]
    alg2 .histoList = [ "1", "2" , "3" , "7" , "8" , "9" , "10" , "11" ]
    alg3 .histoList = [ "1", "2" , "3" , "7" , "8" , "9" , "10" , "11" ]
    alg4 .histoList = [ "1", "2" , "3" , "4" , "7", "8" , "9"]
    
    
    alg.Members = [ alg1 , alg2 , alg3 , alg4 ]
    

    setTheProperty ( alg , 'Context' , context )
    
    return alg





# =============================================================================
## Define pi0 monitoring
def pi0sMoni ( context ) :
    """    
    Define pi0 monitoring
    
    """
    
    alg = getAlgo  ( CaloPi0Monitor   ,
                     'ResolvedPi0Mon' ,
                     context          )
    
    alg.PhotonPtFilter = 500.*MeV;

    alg.histoList = [ "1", "2" , "3" , "4" ]

    setTheProperty ( alg , 'Context' , context )
    
    return alg

# =============================================================================
## define CaloPIDs monitoring
def pidsMoni ( context ) :
    """
    Define CaloPIDs monitoring
    """

    alg = getAlgo ( GaudiSequencer ,
                    'CaloPIDsMoni' ,
                    context        )

    alg1 = getAlgo ( CaloClusterMatchMonitor  ,
                     'PhotonMatchMon'         ,
                     context                  )
    alg2 = getAlgo ( CaloHypoMatchMonitor     ,
                     'ElectronMatchMon'       ,
                     context                  )
    alg3 = getAlgo ( CaloHypoMatchMonitor     ,
                     'BremMatchMon'           ,
                     context                  )
    alg4 = getAlgo ( CaloEMuMonitor           ,
                     'CaloPIDMon'             ,
                     context                  )


    alg1.Input  =   'Rec/Calo/ClusterMatch'
    alg1.Inputs = [ 'Rec/Calo/EcalClusters' ]
    
    alg2.Input  =   'Rec/Calo/ElectronMatch'
    alg2.Inputs = [ 'Rec/Calo/Electrons'    ]
    
    alg3.Input  =   'Rec/Calo/BremMatch'
    alg3.Inputs = [ 'Rec/Calo/Photons'      ]
    
    
    alg.Members = [ alg1 , alg2 , alg3 , alg4 ]
    

    setTheProperty ( alg , 'Context' , context )
    
    return alg
  
    
# =============================================================================
if '__main__' == __name__ :
    print __doc__
    
# =============================================================================
# The END
# =============================================================================
