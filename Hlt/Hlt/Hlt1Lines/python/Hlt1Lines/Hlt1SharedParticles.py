from Configurables import HltLinesConfigurableUser
class Hlt1SharedParticles(HltLinesConfigurableUser):
    __slots__ = {'PT'              : 500,     # MeV
                 'P'               : 3000,
                 'TrackChi2DOF'    : 4.
      }

    def protoParticleUnit(self) :
        selection = 'Hlt1ProtoParticles'
        props = self.getProps().copy()
        props.update({'selection' : selection})
        code = """
        TrackCandidates
        >>  FitTrack
        >>  ( ( TrPT > %(PT)s * MeV ) & \
            ( TrP  > %(P)s  * MeV ) )
        >>  tee  ( monitor( TC_SIZE > 0, '# pass TrackFit', LoKi.Monitoring.ContextSvc ) )
        >>  tee  ( monitor( TC_SIZE    , 'nFit' , LoKi.Monitoring.ContextSvc ) )
        >>  ( TrCHI2PDOF < %(TrackChi2DOF)s )
        >>  tee  ( monitor( TC_SIZE > 0, '# pass TrackChi2', LoKi.Monitoring.ContextSvc ) )
        >>  tee  ( monitor( TC_SIZE    , 'nChi2' , LoKi.Monitoring.ContextSvc ) )
        >>  TC_TOPROTOPARTICLES( '' )
        >>  SINK(  '%(selection)s' )
        >>  ~TC_EMPTY
        """ % props

        from Configurables import LoKi__HltUnit as HltUnit
        from HltTracking.Hlt1Tracking import ( TrackCandidates, FitTrack)
        preambulo = [ TrackCandidates('SharedParticles'),
                      FitTrack ]
        protoUnit = HltUnit(
            'Hlt1ProtoParticleUnit',
            #OutputLevel = 1,
            Monitor = True,
            Preambulo = preambulo,
            Code = code)

        from HltLine.HltLine import bindMembers
        bm = bindMembers(None, [protoUnit]).setOutputSelection(selection)
        return bm

    def pionUnit(self):
        protoUnit = self.protoParticleUnit()
        selection = 'Hlt1SharedPions'
        props = {'selection' : selection,
                 'input'     : protoUnit.outputSelection()}
        code = """
        SELECTION( '%(input)s' )
        >>  TC_TOPARTICLES( 'pi+', '', ALL )
        >>  tee ( monitor( TC_SIZE > 0, '# pass ToPions', LoKi.Monitoring.ContextSvc ) )
        >>  tee ( monitor( TC_SIZE    , 'nPions',         LoKi.Monitoring.ContextSvc ) )
        >>  SINK(  '%(selection)s' )
        >>  ~TC_EMPTY
        """ % props

        from Configurables import LoKi__HltUnit as HltUnit
        pionUnit = HltUnit(
            'Hlt1SharedPionUnit',
            Monitor = True,
            Code = code)

        from HltLine.HltLine import bindMembers
        bm = bindMembers(None, [protoUnit, pionUnit]).setOutputSelection(selection)
        return bm

    def kaonUnit(self):
        protoUnit = self.protoParticleUnit()
        selection = 'Hlt1SharedKaons'
        props = {'selection' : selection,
                 'input'     : protoUnit.outputSelection()}
        code = """
        SELECTION( '%(input)s' )
        >>  TC_TOPARTICLES( 'K+', '', ALL )
        >>  tee ( monitor( TC_SIZE > 0, '# pass ToKaons', LoKi.Monitoring.ContextSvc ) )
        >>  tee ( monitor( TC_SIZE    , 'nKaons',         LoKi.Monitoring.ContextSvc ) )
        >>  SINK(  '%(selection)s' )
        >>  ~TC_EMPTY
        """ % props

        from Configurables import LoKi__HltUnit as HltUnit
        kaonUnit = HltUnit(
            'Hlt1SharedKaonUnit',
            Monitor = True,
            Code = code)

        from HltLine.HltLine import bindMembers
        bm = bindMembers(None, [protoUnit, kaonUnit]).setOutputSelection(selection)
        return bm
