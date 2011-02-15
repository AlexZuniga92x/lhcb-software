##############################
#     HLT Lumi Line(s)
##############################

from Gaudi.Configuration import WARNING,DEBUG
# from GaudiConf.Configuration import *
from HltLine.HltLinesConfigurableUser import HltLinesConfigurableUser


####### create binders...
def _createCounter( counterKind, seqName, seq, enableNonL0Counters ) :
    from Configurables import LumiFromL0DU
    if counterKind is LumiFromL0DU:
        def _fun (name, value ) :
          from HltLine.HltDecodeRaw import DecodeL0DU
          if [ i for i in DecodeL0DU.members() if i not in seq.Members ] :
              seq.Members +=  DecodeL0DU.members() 
          alg = LumiFromL0DU(  seqName + 'L0DU' )
          if alg not in seq.Members :
               alg.InputSelection='Trig/L0/L0DUReport'
               alg.OutputContainer='Hlt/LumiSummary' 
               seq.Members += [ alg ]
          if name in alg.CounterMap :
              raise KeyError('Key %s already present'%name)
          alg.CounterMap.update( { name : value } ) 
        return _fun
    return lambda name, inputSel : seq.Members.append( counterKind( seqName + name
                                                     , InputSelection = inputSel
                                                     , CounterName = name
                                                     , Enable = enableNonL0Counters
                                                     , OutputContainer='Hlt/LumiSummary' ) )

####### operator, meet arguments...
def _combine( op, arg ) :
    for key,value in arg.iteritems() : op(key,value)
    return arg.keys()

############# start building the lumi line(s)...
class Hlt1LumiLinesConf(HltLinesConfigurableUser) :

    from Configurables import ( LumiCountVertices,
                                LumiCountTracks,
                                LumiFromL0DU,
                                LumiCountHltTracks,
                                LumiFlagMethod
                                )
    from HltTracking.HltReco import MinimalVelo
    fastVeloContainer = MinimalVelo.outputSelection()
    __slots__ = { 'TriggerType'            : 'LumiTrigger'  # ODIN trigger type accepted for Lumi
                , 'L0Channel'              : ['CALO']     # L0 channels accepted for LowLumi
                , 'L0MidChannel'           : ['MUON,minbias']     # L0 channels accepted for MidLumi
                , 'LumiLines'              : ['Count','VDM']
                , 'EnableReco'             : True 
                , 'MaxRate'                : 997. # pick a prime number...
                , 'LumiLowRateLimits'      : { 'BeamCrossing' : 'RATE(70)'
                                             , 'Beam1'        : 'RATE(15)'
                                             , 'Beam2'        : 'RATE(10)'
                                             , 'NoBeam'       : 'RATE( 5)'
                                             }
                , 'Postscale'              : { 'Hlt1LumiLow.*RateLimited' : 1.0 }
                , 'CounterDefinition' : { 'RZVelo'   : [LumiCountTracks   , True    ,  fastVeloContainer,   5,  200]
                                        , 'Muon'     : [LumiCountTracks   , False   , 'Hlt/Track/Muons' ,   5,  200]
                                        , 'TTIP'     : [LumiCountTracks   , True    , 'Hlt/Track/TTIP'  ,   5,  100]
                                        , 'TTMIB'    : [LumiCountTracks   , False   , 'Hlt/Track/TTMIB' ,   5,  100]
                                        , 'PV3D'     : [LumiCountVertices , True    , 'Hlt/Vertex/PV3D' ,   1,   20]
                                        , 'RZVeloBW' : [LumiCountHltTracks, True    , 'VeloBW'          ,   5,  200]
                                        , 'SPDMult'  : [LumiFromL0DU      , True    , 'Spd(Mult)'       ,   6,  500]
                                        , 'PUMult'   : [LumiFromL0DU      , True    , 'PUHits(Mult)'    ,   3,  200]
                                        , 'CaloEt'   : [LumiFromL0DU      , True    , 'Sum(Et)'         , 500, 6000]
                                        }
                , 'OutputLevel'            : WARNING
                }

    def __create_lumi_algos__(self, postfix=''):
        '''
        returns algorithm sequences for Hlt1 Lumi Lines
        '''
        # get counters
        counters = self.getProp('CounterDefinition')
        
        # debugging options
        debugOPL = self.getProp('OutputLevel')
        from HltTracking.HltPVs  import PV3D
        # define reco scaler
        from Configurables import DeterministicPrescaler as Scaler
        recoScaler = Scaler( 'LumiRecoScaler' ,  AcceptFraction = 1 if self.getProp('EnableReco') else 0 )  

        # define empty reco sequence
        seqRecoName = 'LumiReco'

        from Configurables import GaudiSequencer as Sequence
        lumiRecoSequence = Sequence(seqRecoName+'Seq'
                                    , ModeOR = True
                                    , ShortCircuit = False
                                    , OutputLevel = debugOPL
                                    , IgnoreFilterPassed = True
                                    , Members = [] # reset so we build the same things several times TODO: move out of loop...
                                    , MeasureTime = True)

        # define empty sequence to collect counters
        seqCountName = 'LumiCount'+postfix
        from Configurables import GaudiSequencer as Sequence
        lumiCountSequence = Sequence(seqCountName +'Seq'
                                    , ModeOR = True
                                    , ShortCircuit = False
                                    , OutputLevel = debugOPL
                                    , IgnoreFilterPassed = True
                                    , MeasureTime = True)

        # LumiLow lines must be flagged - traditional and low are flagged explicitely
        from Configurables import LumiFlagMethod
        if postfix.find('Low') > -1  :
            lumiCountSequence.Members.append( LumiFlagMethod( seqCountName+'FlagMethod'
                                                              , CounterName='Method'
                                                              , ValueName='L0RateMethod'
                                                              , OutputContainer='Hlt/LumiSummary' ) )
            
        # LumiMid lines are not flagged - to be used for on-line reporting only
        elif postfix.find('Mid') > -1  :
            pass
            
        # flag now also the random lumi lines - needed due to microbias lines
        else  :
            lumiCountSequence.Members.append( LumiFlagMethod( seqCountName+'FlagMethod'
                                                              , CounterName='Random'
                                                              , ValueName='RandomMethod'
                                                              , OutputContainer='Hlt/LumiSummary' ) )
            
        # populate count sequence from the definition
        createdCounters = []
        histoThresholds = []
        histoMaxBins = []
        for key, definition in counters.iteritems():
            # example 'CaloEt' : [LumiFromL0DU , True , 'Sum(Et)' , 500, 6000],
            (op, flag, inputDef, threshold, bins) = definition
            if flag:
                createdCounters.extend( 
                    _combine( _createCounter( op, seqCountName, lumiCountSequence, True ),
                              { key : inputDef } ) )
                histoThresholds.extend( [threshold] )
                histoMaxBins.extend( [bins] )
                if debugOPL <= DEBUG:
                    print '# DEBUG   : Hlt1LumiLines::HistoMaker:', postfix, key, threshold, bins
                
        lumiRecoSequence.Members.append( Sequence('LumiTrackRecoSequence' ,
                                                   Members = [  recoScaler ] + PV3D().members(),
                                                   MeasureTime = True ) ) 

        # filter to get backward tracks (make sure it always passes by wrapping inside a sequence)
        from Configurables import Hlt__TrackFilter as HltTrackFilter
        from Configurables import GaudiSequencer as Sequence
        lumiRecoFilterSequence = Sequence( 'LumiRecoFilterSequence', Members = [] ) # reset, always build the same seq...
        lumiRecoFilterSequence.Members.append( recoScaler )
        from HltTracking.HltReco import MinimalVelo
        fastVeloContainer = MinimalVelo.outputSelection()
        lumiRecoFilterSequence.Members.append(
            Sequence('HltVeloBWSequence'
                     , Members  = [ HltTrackFilter('HltPrepareVeloBW'
                                                   , InputSelection   = fastVeloContainer
                                                   , Code = [ 'TrBACKWARD' ]
                                                   , OutputSelection     = 'VeloBW'
                                                   , RequirePositiveInputs = False
                                                   ) ]
                     , MeasureTime = True
                     , ModeOR = True
                     , ShortCircuit = False
                     ) )
        lumiRecoSequence.Members.append(lumiRecoFilterSequence)
        
        # sequence to get TT tracks
        #  disabled because of https://savannah.cern.ch/bugs/index.php?62933
        #from HltLine.HltDecodeRaw import DecodeTT
        #from Configurables import TTGenericTracking
        #from Configurables import RawBankToSTClusterAlg
        #lumiTTTSequence = Sequence( 'LumiTTTSequence', Members = [] ) # reset, always build the same seq...
        #lumiTTTSequence.Members.append( recoScaler )
        #lumiTTTSequence.Members.append(
        #    Sequence('TTTSequence'
        #             # , Members  = DecodeTT.members() + [
        #             , Members  = [ RawBankToSTClusterAlg("createTTClusters"),
        #                            TTGenericTracking("lumiTTT"
        #                                             , MaxNumClusters = 2000
        #                                             # , InputData = DecodeTT.members()[0].getProp('clusterLocation') // hardwired to use STClusters, not lite ones...
        #                                             , OutputLocation = 'Hlt/Track/TTIP'
        #                                             , WindowCenter = [0,0,0]
        #                                             , HalfWindowXSize = 15
        #                                             , HalfWindowYSize = 15
        #                                             , OutputLevel = WARNING
        #                                              )
        #                            ]
        #             , MeasureTime = True
        #             , ModeOR = True
        #             , ShortCircuit = False
        #             ) )
        #lumiRecoSequence.Members.append(lumiTTTSequence)

        # define histogrammers
        #from Configurables import LumiHistoMaker, LumiHisto2dSPD
        #HistoMembers=[]
        #HistoMembers.append(LumiHistoMaker('Histo'+postfix
        #                                   InputVariables = createdCounters,
        #                                   Thresholds = histoThresholds,
        #                                   MaxBins = histoMaxBins,
        #                                   OutputLevel = debugOPL
        #                                   ))
        #HistoMembers.append(LumiHisto2dSPD('Histo2D'+postfix
        #                                   HistoTitle=str(postfix),
        #                                   OutputLevel = debugOPL ))

        #lumiHistoSequence = Sequence('LumiHisto'+postfix+'Seq'
        #                             , Members = HistoMembers
        #                             , ModeOR = True
        #                             , ShortCircuit = False
        #                             , MeasureTime = True
        #                             , OutputLevel = debugOPL
        #                             )

        #return [ lumiRecoSequence, lumiCountSequence, lumiHistoSequence ] 
        return [ lumiRecoSequence, lumiCountSequence ] 


    def __create_lumi_line__(self ):
        '''
        returns an Hlt1 "Line" including input filter, reconstruction sequence and counting
        adds histogramming
        '''
        from HltLine.HltLine import Hlt1Line   as Line
        from Configurables import LoKi__Hybrid__HltFactory as HltFactory
        for i in [ 'LoKiCore.functions', 'LoKiNumbers.sources' ] :
            if i not in HltFactory('ToolSvc.HltFactory').Modules : HltFactory('ToolSvc.HltFactory').Modules += [ i ]
        return Line ( 'Lumi'
                    , prescale = self.prescale
                    , ODIN = 'scale( ODIN_TRGTYP == LHCb.ODIN.%s , RATE(%s,LoKi.Scalers.RandomPhasePeriodicLimiter)) ' % ( self.getProp('TriggerType'), self.getProp('MaxRate') )
                    , algos = self.__create_lumi_algos__( '' )
                    , postscale = self.postscale
                    ) 

    def __create_lumi_low_line__(self, BXType, extension = '' ):
        '''
        returns an Hlt1 "Line" including input filter, reconstruction sequence and counting
        adds histogramming
        '''
        postfix = 'Low'
        from HltLine.HltLine import Hlt1Line   as Line
        l0du = ' | '.join( [ (" ( L0_CHANNEL('%s') ) "%(x)) for x in  self.getProp('L0Channel') ] )
        return Line ( 'Lumi'+postfix+BXType + extension
                    , prescale = self.prescale
                    , ODIN = ' ( ODIN_BXTYP == LHCb.ODIN.%s ) ' % BXType
                    , L0DU  = l0du
                    , algos = self.__create_lumi_algos__( postfix+BXType )
                    , postscale = self.postscale
                    ) 

    def __create_lumi_mid_line__(self, BXType='BeamCrossing', extension = '' ):
        '''
        returns an Hlt1 "Line" including input filter, reconstruction sequence and counting
        adds histogramming
        '''
        postfix = 'Mid'
        from HltLine.HltLine import Hlt1Line   as Line
        l0du = ' | '.join( [ (" ( L0_CHANNEL('%s') ) "%(x)) for x in  self.getProp('L0MidChannel') ] )
        return Line ( 'Lumi'+postfix+BXType + extension
                    , prescale = self.prescale
                    , ODIN = ' ( ODIN_BXTYP == LHCb.ODIN.%s ) ' % BXType
                    , L0DU  = l0du
                    , algos = self.__create_lumi_algos__( postfix+BXType )
                    , postscale = self.postscale
                    ) 

    def __apply_configuration__(self) :
        '''
        creates parallel HLT1 Lines for each beam crossing type
        '''
        from HltLine.HltLine import Hlt1Line   as Line
        # LumiTrigger lines
        self.__create_lumi_line__()
        # PhysicsTrigger lines
        lines = map( self.__create_lumi_low_line__, ['NoBeam', 'BeamCrossing','Beam1','Beam2'] )
        for i in lines :
            i.clone( i.name().lstrip('Hlt1')+'RateLimited'
                   , L0DU = 'scale( %s , %s)' % ( i._L0DU , self.getProp('LumiLowRateLimits')[i.name().lstrip('Hlt1LumiLow')] ) 
                   , postscale = self.postscale
                   , prescale = self.prescale
                   )
        # PhysicsTrigger lines for higher rates
        lines = map( self.__create_lumi_mid_line__, ['BeamCrossing'] )
        #
        # NOTE: this line should not be rate-limited, but a fixed prescale can be used if needed.
        #

