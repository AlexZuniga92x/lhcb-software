class DiMuonLines :
    """
    Threshold settings for Hlt1 and 2 in nominal conditions (list of exclusive)
    
    WARNING :: DO NOT EDIT WITHOUT PERMISSION OF THE AUTHORS
    
    @author P.Koppenburg, M. Gandelman, G. Raven
    @date 2009-08-05
    """
    
    __all__ = ( 'ActiveHlt2Lines' )
    
    
    def ActiveHlt2Lines(self) :
        """
        Returns a list of active lines
        """

        lines = [

            'Hlt2MuonFromHLT1',
            'Hlt2SingleMuon',
            'Hlt2SingleHighPTMuon',
            'Hlt2MuTrack',
            'Hlt2MuTrackNoIP',
            'Hlt2DiMuonUnbiasedJPsi',
            'Hlt2DiMuonUnbiasedPsi2S',
            'Hlt2DiMuonUnbiasedBmm',
            'Hlt2DiMuonUnbiasedZmm',
            'Hlt2DiMuonDY1',
            'Hlt2DiMuonDY2',
            'Hlt2DiMuonDY3',
            'Hlt2DiMuonDY4',
            'Hlt2BiasedDiMuonSimple',
            'Hlt2BiasedDiMuonRefined',
            'Hlt2BiasedDiMuonMass',
            'Hlt2BiasedDiMuonIP',
#            'Hlt2DiMuonUnbiasedJPsiLow',
            'Hlt2UnbiasedDiMuon',  
            'Hlt2UnbiasedDiMuonLowMass',
            'Hlt2DiMuonSameSign'
            ]
            
        return lines

   
    def Thresholds(self) :
        """
        Returns a dictionary of cuts
        """
        
        # keep pass through thresholds
        d = { }

        from Hlt2Lines.Hlt2InclusiveMuonLines  import Hlt2InclusiveMuonLinesConf
        from Hlt2Lines.Hlt2InclusiveDiMuonLines  import Hlt2InclusiveDiMuonLinesConf
        d.update( { Hlt2InclusiveDiMuonLinesConf : { 
            ## Cut values
            'UnbiasedDiMuonPt'         : 0.0      # in MeV
            ,'UnbiasedDiMuonMuPt'      : 0.0      # in MeV
            ,'UnbiasedJPsiPt'          : 0.0      # MeV
            ,'UnbiasedJPsiMassWindow'  : 120      # MeV
            ,'UnbiasedPsi2SPt'         : 0.0      # MeV
            ,'UnbiasedPsi2SMuPt'       : 500      # MeV
            ,'UnbiasedPsi2SMassWindow' : 120      # MeV
            ,'UnbiasedBmmMinMass'      : 5000     # MeV
            ,'DYPt'                    : 0.0      # MeV
            ,'DY1MinMass'              : 2500      # MeV
            ,'DY1MaxMass'              : 6000      # MeV
            ,'DY2MinMass'              : 5000      # MeV
            ,'DY2MaxMass'              :12000      # MeV
            ,'DY3MinMass'              :10000      # MeV
            ,'DY3MaxMass'              :25000      # MeV
            ,'DY4MinMass'              :20000      # MeV
            ,'DY4MaxMass'              :50000      # MeV
            ,'UnbiasedZmmMinMass'      :40000      # MeV
            ,'UnbiasedZmmPt'           :  0.0      # MeV
            , 'Prescale'   : { 'Hlt2UnbiasedDiMuon'          : 1.00
                               , 'Hlt2UnbiasedDiMuonLowMass' : 0.1
                               ,'Hlt2DiMuonDY1'              :  1
                               ,'Hlt2DiMuonDY2'              :  1
                               ,'Hlt2DiMuonSameSign'         :  1
                               }           
            }}
                  )

        d.update( { Hlt2InclusiveMuonLinesConf : {
            'SingleMuonPt'        : 1000      # MeV
            ,'SingleMuonIP'        : 0.08     # mm
            ,'SingleMuonHighPt'    : 10000     # MeV
            ,'MuTrackMuPt'         : 1000      # MeV
            ,'MuTrackTrPt'         : 600       # MeV
            ,'MuTrackMuIP'         : 0.08     # mm
            ,'MuTrackTrIP'         : 0.08     # mm
            ,'MuTrackDoca'         : 0.2     # mm
            ,'MuTrackDz'           : 1.0       # mm
            ,'MuTrackMass'         : 2000      # MeV
            ,'MuTrackPoint'        : 0.4       # dimensionless
            ,'MuTrackNoIPMuPt'    : 1600      # MeV
            ,'MuTrackNoIPTrPt'    : 400       # MeV
            ,'MuTrackNoIPDoca'    : 0.100     # mm
            ,'MuTrackNoIPMass'    : 2900      # MeV
            ,'Prescale'   : { 'Hlt2MuonFromHLT1'       : 0.01   
                              ,'Hlt2SingleMuon'        : 0.02
                              ,'Hlt2SingleHighPTMuon'  : 1.0
                              ,'Hlt2IncMuTrack'        : 1.0
                              ,'Hlt2IncMuTrackNoIP'    : 1.0
                              }
            }}
                  )
        
        return d
    
