"""
Default configuration for stripping DST.
"""

__author__ = "Juan Palacios <juan.palacios@nikhef.nl>"

__all__ = ('stripDSTElements',
           'stripDSTStreamConf')

from Configurables import InputCopyStream
from streamconf import OutputStreamConf
from microdstelements import ( CloneParticleTrees,
                               ClonePVRelations,
                               PackStrippingReports,
                               PackParticlesAndVertices,
                               PackRecObjects,
                               CleanEmptyEventNodes )

def stripDSTElements(pack=True) :
    elements = [ CloneParticleTrees( ProtoParticleConer = "NONE" ),
                 ClonePVRelations( location = "Particle2VertexRelations",
                                   clonePVs = True,
                                   RecVertexCloner = "VertexBaseFromRecVertexCloner" )
                 ]
    if pack : elements += [ PackStrippingReports(),
                            PackParticlesAndVertices(),
                            PackRecObjects(),
                            CleanEmptyEventNodes() ]
    return elements

def stripDSTStreamConf( pack = True,
                        saveFullRawEvent = True ) :
    eItems = [ ]
    if saveFullRawEvent :
        eItems += [ '/Event/DAQ/RawEvent#1' ]
    if pack :
        eItems += [ '/Event/Strip/pPhys/DecReports#1' ]
    else :
        eItems += [ '/Event/Strip/Phys/DecReports#1' ]
    return OutputStreamConf( streamType = InputCopyStream,
                             extraItems = eItems )
