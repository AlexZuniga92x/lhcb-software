class Physics_pAPilot :
    """
    Threshold settings for the pilot pA run in September 2012
    
    WARNING :: DO NOT EDIT WITHOUT PERMISSION OF THE AUTHORS
    
    @author Johannes Albrecht
    @date 2012-08-27
    """
    __all__ = ( 'ActiveHlt1Lines', 'ActiveHlt2Lines', 'Thresholds','L0TCK' )

    def verifyType(self,ref) :
        # verify self.ActiveLines is still consistent with
        # our types self.ActiveHlt2Lines and self.ActiveHlt1Lines...
        # so we can force that classes which inherit from us
        # and overrule either ActiveHlt.Lines also overrule
        # HltType...
        if ( self.ActiveHlt1Lines() != ref.ActiveHlt1Lines(self)  or
             self.ActiveHlt2Lines() != ref.ActiveHlt2Lines(self) ) :
            raise RuntimeError('Must update HltType when modifying ActiveHlt.Lines()')

    def HltType(self) :  
        self.verifyType( Physics_pAPilot )
        return          'Physics_pAPilot'
    
    def __init__(self) :
#        self.StripEndSequence = ['']
        self.NanoBanks = ['ODIN', 'HltLumiSummary', 'HltRoutingBits', 'DAQ', 'Velo']

    def L0TCK(self) :
        return '0x1710'

    def ActiveHlt2Lines(self) :
        """
        Returns a list of active lines
        """
        return  [ 'Hlt2PassThrough','Hlt2Transparent' ]
   
    def ActiveHlt1Lines(self) :
        """
        Returns a list of active lines
        """
        return [ 'Hlt1L0Any',
                 'Hlt1ActivitySPD',
                 'Hlt1ActivityTracking',
#  doesn't work online		 
#                 'Hlt1MicroBiasVeloLowMult',
                 'Hlt1MBNoBias',
                 'Hlt1Lumi',
                 ]

    def Thresholds(self) :
        """
        Returns a dictionary of cuts
        """
        from Hlt1Lines.Hlt1L0Lines import Hlt1L0LinesConf
        from Hlt1Lines.Hlt1MBLines import Hlt1MBLinesConf
        from Hlt1Lines.Hlt1LumiLines import Hlt1LumiLinesConf
        from Hlt2Lines.Hlt2CommissioningLines import Hlt2CommissioningLinesConf
        
        return { Hlt1L0LinesConf : { 'Prescale' : { 'Hlt1L0Any'    : 1.}}
               , Hlt1LumiLinesConf : { 'EnableReco' : False }
               , Hlt1MBLinesConf : {  'Prescale' : { 'Hlt1MBNoBias'  : 1. }
                                      , 'Postscale' : { 'Hlt1MBNoBias'  : 'RATE(1000)' }}
               , Hlt2CommissioningLinesConf : {'Prescale' : { 'Hlt2PassThrough'  : 1. }}
               }
