"""
Write a DST for a single selection sequence. Writes out the entire
contents of the input DST
"""
__version__ = "$Id: SelDSTWriter.py,v 1.4 2010-08-05 10:14:53 jpalac Exp $"
__author__ = "Juan Palacios <juan.palacios@nikhef.nl>"

__all__ = ('selDSTElements',
           'selDSTStreamConf')

from GaudiConf.Configuration import *
from Configurables import InputCopyStream

from streamconf import OutputStreamConf
from microdstelements import CloneParticleTrees, ClonePVRelations

def selDSTElements() :
    return[CloneParticleTrees(copyProtoParticles = False),
           ClonePVRelations("Particle2VertexRelations",True)]

def selDSTStreamConf() :
    return OutputStreamConf(streamType = InputCopyStream)
