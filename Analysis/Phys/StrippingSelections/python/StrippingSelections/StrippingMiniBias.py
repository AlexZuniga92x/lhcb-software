#
# Stripping selections 
# for Minimum Bias physics.
# author: Yasmine Amhis
# date : 2009-11-11
#
 
# Begin StrippingMinBias.py
from Configurables import LoKi__VoidFilter as VoidFilter
from StrippingConf.StrippingLine import StrippingLine
from Configurables import LoKi__Hybrid__CoreFactory as CoreFactory

modules =  CoreFactory('CoreFactory').Modules
for i in [ 'LoKiTrigger.decorators' ] :
  if i not in modules : modules.append(i)
#----------------------------------------------------------------------
#Here we require at least one  reconstructed best track in the event 

#-----------------------------------------------------------------
# Microbias line
#-----------------------------------------------------------------
MBMicroBiasRateLimitedLine  = StrippingLine( "MBMicroBiasRateLimited"
                                             , HLT = "HLT_PASS_RE('Hlt1MBMicro.*RateLimitedDecision')"
                                             , checkPV = False
                                             , prescale = 1
                                             , postscale = 1)


MBMicroBiasNotRateLimitedLine  = StrippingLine( "MBMicroBiasNotRateLimited"
                                                , HLT = "HLT_PASS_RE('Hlt1MBMicro.*(?<!RateLimited)Decision')"
                                                , checkPV = False
                                                , prescale = 1
                                                , postscale = 1)



#-----------------------------------------------------------------
# No bias line
#-----------------------------------------------------------------
MBNoBiasLine  = StrippingLine( "MBNoBias"
                               , HLT = "HLT_PASS('Hlt1MBNoBiasDecision')"
                               , checkPV = False
                               , prescale = 1
                               , postscale = 1)

#-----------------------------------------------------------------
# Mini bias line
#-----------------------------------------------------------------
MBMiniBiasLine  = StrippingLine( "Hlt1L0Any"
                                 , HLT = "(HLT_PASS_RE('Hlt1L0Any.*Decision'))" 
                                 , checkPV = False
                                 , prescale = 1
                                 , postscale = 1)


# End StrippingMiniBias.py

