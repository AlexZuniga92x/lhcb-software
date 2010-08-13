#
# @todo Move me to LHCB !
#
from Configurables import DataOnDemandSvc
from GaudiKernel.ProcessJobOptions import importOptions
from HltLine import bindMembers
importOptions('$STDOPTS/DecodeRawEvent.py')

def _convert(map, name) :
    x = map.pop(name)
    a = x.split('/')
    if len(a) == 2 :
        (t,n) = a
    elif len(a) == 1 :
        (t,n) = (a[0],a[0])
    cls = getattr( __import__('Configurables'),t )
    return bindMembers( None, [ cls(n) ]).ignoreOutputSelection()

# bind __convert to _convert
__convert = lambda x : _convert( DataOnDemandSvc().AlgMap,x) 

# __convert = lambda x : bindMembers( None, [] ).ignoreOutputSelection()


DecodeODIN   = __convert( 'DAQ/ODIN' )
DecodeL0MUON = __convert( 'Trig/L0/MuonCtrl' )
DecodeL0CALO = __convert( 'Trig/L0/Calo' )
__convert(  'Trig/L0/FullCalo')
DecodeL0DU   = __convert( 'Trig/L0/L0DUReport' )
DecodeVELO   = __convert( 'Raw/Velo/LiteClusters' )
DecodeTT     = __convert( 'Raw/TT/LiteClusters' )
DecodeIT     = __convert( 'Raw/IT/LiteClusters' )
DecodeOT     = __convert( 'Raw/OT/Times' )
#decodeMUON  = __convert( 'Raw/Muon/Coords' )  # How can we access the MUON data if this is commented out??? (and Dod is not running!)
#decodeRICH  = __convert( 'Raw/Rich/Digits' )  # How can we access the RICH data if this is commented out??? (and Dod is not running!)
DecodeECAL   = __convert( 'Raw/Ecal/Digits' )
DecodeSPD    = __convert( 'Raw/Spd/Digits' )
DecodePRS    = __convert( 'Raw/Prs/Digits' )
DecodeHCAL   = __convert( 'Raw/Hcal/Digits' )


#and export explicit (configured!) decoders:
__all__ = ( 'DecodeL0DU', 'DecodeL0MUON', 'DecodeL0CALO'
          , 'DecodeVELO', 'DecodeTT', 'DecodeIT', 'DecodeOT'
          , 'DecodeECAL', 'DecodeSPD', 'DecodePRS', 'DecodeHCAL' )

