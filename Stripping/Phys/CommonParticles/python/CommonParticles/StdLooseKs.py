#!/usr/bin/env python
# =============================================================================
# $Id: StdLooseKs.py,v 1.5 2009-05-26 13:48:13 gcowan Exp $ 
# =============================================================================
## @file  CommonParticles/StdLooseKs.py
#  configuration file for 'Standard Loose ' 
#  @author Patrick Koppenburg
#  @date 2009-02-18
# =============================================================================
"""
Configuration file for 'Standard Loose Ks'
"""
__author__  = "Patrick Koppenburg"
__version__ = "CVS tag $Name: not supported by cvs2svn $, version $Revision: 1.5 $"
# =============================================================================
__all__ = (
    'StdLooseKsLL' ,
    'StdLooseKsDD' ,
    'locations'
    )
# =============================================================================
from Gaudi.Configuration import *
from Configurables       import CombineParticles
from CommonParticles.Utils import *

## ============================================================================
## create the algorithm 
StdLooseKsLL = CombineParticles ( 'StdLooseKsLL' )

StdLooseKsLL.Inputs = [ "Phys/StdLoosePions/Particles" ]
StdLooseKsLL.DecayDescriptor = "KS0 -> pi+ pi-" 

StdLooseKsLL.DaughtersCuts = { "pi+" : "(P > 2.*GeV) & (MIPCHI2DV(PRIMARY) > 9.)" } 
StdLooseKsLL.CombinationCut = "(ADAMASS('KS0') < 50.*MeV) & (ADOCACHI2CUT(25, '')) "
StdLooseKsLL.MotherCut = "(ADMASS('KS0') < 35.*MeV) & (VFASPF(VCHI2) < 25.)"

## configure Data-On-Demand service 
locations = updateDoD ( StdLooseKsLL )

## ============================================================================
#  configuration file for 'Standard Loose Downstream ' 
#  @author Greig Cowan
#  @date 2009-04-27
# ============================================================================= 
StdLooseKsDD = CombineParticles ( 'StdLooseKsDD' )

StdLooseKsDD.Inputs = [ "Phys/StdNoPIDsDownPions/Particles" ]
StdLooseKsDD.DecayDescriptor = "KS0 -> pi+ pi-" 

StdLooseKsDD.DaughtersCuts = { "pi+" : "(P > 2.*GeV) & (MIPCHI2DV(PRIMARY) > 4.)" } 
StdLooseKsDD.CombinationCut = "(ADAMASS('KS0') < 80.*MeV) & (ADOCACHI2CUT(25, ''))"
StdLooseKsDD.MotherCut = "(ADMASS('KS0') < 64.*MeV) & (VFASPF(VCHI2) < 25.)"

## configure Data-On-Demand service 
locations.update( updateDoD ( StdLooseKsDD ))

## ============================================================================
if '__main__' == __name__ :

    print __doc__
    print __author__
    print __version__
    print locationsDoD ( locations ) 

# =============================================================================
# The END 
# =============================================================================
