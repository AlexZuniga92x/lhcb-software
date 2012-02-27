class CharmHadronLines :
    """
    Threshold settings for Hlt1 and 2 in nominal conditions (list of exclusive)
    
    WARNING :: DO NOT EDIT WITHOUT PERMISSION OF THE AUTHORS
    
    @author P.Koppenburg, M. Gandelman, G. Raven, V. Gligorov, P. Spradlin
    @date 2009-08-05
    """
    
    __all__ = ( 'ActiveHlt2Lines' )
    
    
    def ActiveHlt2Lines(self) : # {
        """
        Returns a list of active lines
        """

        lines = [ ### from Hlt2CharmHadD02HHLines
                  'Hlt2CharmHadD02HH_D02KK'
                  , 'Hlt2CharmHadD02HH_D02KPi'
                  , 'Hlt2CharmHadD02HH_D02PiPi'
                  ## Inclusive lines
                  #, 'Hlt2CharmHadD02HH2BodyInc'
                  ## Wide mass lines
                  , 'Hlt2CharmHadD02HH_D02KKWideMass'
                  , 'Hlt2CharmHadD02HH_D02KPiWideMass'
                  , 'Hlt2CharmHadD02HH_D02PiPiWideMass'

                  ### from Hlt2CharmHadD2HHHLines
                  , 'Hlt2CharmHadD2HHH'
                  , 'Hlt2CharmHadD2HHHWideMass',

                  ### from Hlt2CharmHadD02HHHH(Dst)Lines
                  'Hlt2CharmHadD02HHHH_4pi',
                  'Hlt2CharmHadD02HHHH_4piWideMass',
                  'Hlt2CharmHadD02HHHH_K3pi',
                  'Hlt2CharmHadD02HHHH_K3piWideMass',
                  'Hlt2CharmHadD02HHHH_KKpipi',
                  'Hlt2CharmHadD02HHHH_KKpipiWideMass',
                  'Hlt2CharmHadD02HHHH_2K2pi',
                  'Hlt2CharmHadD02HHHH_2K2piWideMass',
                  'Hlt2CharmHadD02HHHH_3Kpi',
                  'Hlt2CharmHadD02HHHH_3KpiWideMass',
                  'Hlt2CharmHadD02HHHH_Ch2',
                  'Hlt2CharmHadD02HHHH_Ch2WideMass',
                  'Hlt2CharmHadD02HHHHDst_4pi',
                  'Hlt2CharmHadD02HHHHDst_4piWideMass',
                  'Hlt2CharmHadD02HHHHDst_K3pi',
                  'Hlt2CharmHadD02HHHHDst_K3piWideMass',
                  'Hlt2CharmHadD02HHHHDst_KKpipi',
                  'Hlt2CharmHadD02HHHHDst_KKpipiWideMass',
                  'Hlt2CharmHadD02HHHHDst_2K2pi',
                  'Hlt2CharmHadD02HHHHDst_2K2piWideMass',
                  'Hlt2CharmHadD02HHHHDst_3Kpi',
                  'Hlt2CharmHadD02HHHHDst_3KpiWideMass',
                  'Hlt2CharmHadD02HHHHDst_Ch2',
                  'Hlt2CharmHadD02HHHHDst_Ch2WideMass'

                  ### from Hlt2CharmHadD02HHKsLines
                  , 'Hlt2CharmHadD02HHKsLL'
                  , 'Hlt2CharmHadD02HHKsDD'

                  ### from Hlt2CharmHadD2KS0HLines
                  , 'Hlt2CharmHadD2KS0H_D2KS0Pi'
                  , 'Hlt2CharmHadD2KS0H_D2KS0K'
                  , 'Hlt2CharmHadD2KS0H_D2KS0DDPi'
                  , 'Hlt2CharmHadD2KS0H_D2KS0DDK'  
                  #LambdaC line
                  , 'Hlt2CharmHadLambdaC2KPPi'  
                  #Minimum bias line
                  , 'Hlt2CharmHadMinBiasLambdaC2KPPi'   
                  , 'Hlt2CharmHadMinBiasD02KPi'
                  , 'Hlt2CharmHadMinBiasD02KK'
                  , 'Hlt2CharmHadMinBiasDplus2hhh'
                  , 'Hlt2CharmHadMinBiasLambdaC2LambdaPi'
             ]
        
        return lines
    # }

    def Thresholds(self) : # {
        """
        Returns a dictionary of cuts
        """
        from GaudiKernel.SystemOfUnits import MeV, GeV, mm


        d = { }

        from Hlt2Lines.Hlt2CharmHadD02HHLines   import Hlt2CharmHadD02HHLinesConf
        d.update( { Hlt2CharmHadD02HHLinesConf       : {
                        ## Cut values for basic D0 -> hh signal lines
                          'Trk_PT_MIN'               : 800.0 * MeV
                        , 'Trk_P_MIN'                : 5.0  * GeV
                        , 'Trk_MIPCHI2DV_MIN'        : 9.0        # neuter
                        , 'Trk_TRCHI2DOF_MAX'        : 3.0        # neuter
                        , 'Pair_AMINDOCA_MAX'        : 0.10 * mm
                        , 'Trk_Max_APT_MIN'          : 1500.0 * MeV
                        , 'D0_BPVVDCHI2_MIN'         : 40.0       # neuter
                        , 'D0_BPVDIRA_MIN'           : 0.99985    # neuter
                        , 'D0_VCHI2PDOF_MAX'         : 10.0       # neuter
                        , 'D0_PT_MIN'                : 2000.0 * MeV
                        , 'Sig_M_MIN'                : 1815.0 * MeV
                        , 'Sig_M_MAX'                : 1915.0 * MeV
                        , 'WideMass_M_MIN'           : 1715.0 * MeV
                        , 'WideMass_M_MAX'           : 2065.0 * MeV
                        ## 2-body inclusive!
                        , 'Inc_Trk_PT_MIN'           : 500.0 * MeV
                        , 'Inc_Trk_P_MIN'            : 5.0 * GeV
                        , 'Inc_Trk_MIPCHI2DV_MIN'    : 16.0       # neuter
                        , 'Inc_Trk_TRCHI2DOF_MAX'    : 3.0        # neuter
                        , 'Inc_Pair_AMINDOCA_MAX'    : 0.15 * mm
                        , 'Inc_D0_BPVVDCHI2_MIN'     : 100.0      # neuter
                        , 'Inc_Trk_Max_APT_MIN'      : 1000.0 * MeV
                        , 'Inc_D0_APT_MIN'           : 1500.0 * MeV
                        , 'Inc_D0_SumAPT_MIN'        : 1800.0 * MeV
                        , 'Inc_D0_BPVIPCHI2_MIN'     : 2.0        # neuter
                        , 'Inc_D0_BPVCORRM_MIN'      : 300.0 * MeV
                        , 'Inc_D0_BPVCORRM_MAX'      : 3200.0 * MeV
                        ## GEC
                        , 'GEC_Filter_NTRACK'        : True       # 
                        , 'GEC_NTRACK_MAX'           : 180        # 
                        , 'Postscale' : { 
                                          'Hlt2CharmHadD02KKWideMass'    : 0.1
                                         , 'Hlt2CharmHadD02KPiWideMass'  : 0.1
                                         , 'Hlt2CharmHadD02PiPiWideMass' : 0.1
                                        }
                    }

                  } )

        return d
    # }
            
   
