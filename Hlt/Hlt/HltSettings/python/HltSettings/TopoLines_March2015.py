class TopoLines_March2015 :
    """
    Threshold settings for Hlt2 topological lines.
    
    WARNING :: DO NOT EDIT WITHOUT PERMISSION!!!!!!!!
    """
    
    __all__ = ( 'ActiveHlt2Lines' )
    
    
    def ActiveHlt2Lines(self) :
        """Returns a list of active lines."""

        lines = ['Hlt2Topo2BodySimple',
                 'Hlt2Topo3BodySimple',
                 'Hlt2Topo4BodySimple',
                 'Hlt2Topo2BodyBBDT',
                 'Hlt2Topo3BodyBBDT',
                 'Hlt2Topo4BodyBBDT',
                 'Hlt2TopoMu2BodyBBDT',
                 'Hlt2TopoMu3BodyBBDT',
                 'Hlt2TopoMu4BodyBBDT',
                 'Hlt2TopoE2BodyBBDT',
                 'Hlt2TopoE3BodyBBDT',
                 'Hlt2TopoE4BodyBBDT',
                 'Hlt2TopoRad2BodyBBDT',
                 'Hlt2TopoRad2plus1BodyBBDT'
                 ]
        
        return lines

   
    
    def Thresholds(self) :
        """Returns a dictionary of cuts."""

        # keep pass through thresholds
        d = { }

        # simple vars
        ptSum = "PTSUM"
        doca = "DOCA"
        ipChi2 = "CANDIPCHI2"
        mCor = "MCOR"
        ptMin = "PTMIN"
        fdChi2 = "FDCHI2"

        # NB: All vars are positive so negative = "no cut"
        from Hlt2Lines.Topo.Lines  import TopoLines
        d.update( { TopoLines : {'Common' : {
            # single track cuts
            'ALL_PT_MIN'        : 500.0,  # MeV
            'ALL_P_MIN'         : 5000.0, # MeV
            'ALL_MIPCHI2DV_MIN' : 4.0,    # unitless
            'MU_TRCHI2DOF_MAX'  : 2.5,    # unitless
            'E_TRCHI2DOF_MAX'   : 2.5,    # unitless
            'HAD_TRCHI2DOF_MAX' : 2.5,    # unitless
            # V0's
            'USE_KS'            : True,
            'USE_LAMBDA'        : True,
            # upfront combo cuts
            'AMAXDOCA_MAX'      : 0.2,    # mm        
            'BPVVDCHI2_MIN'     : 100.0,  # unitless
            'MIN_TRCHI2DOF_MAX' : 2.0,      # unitless
            'ONETRACK_PT_MIN'   : 1500.0, # MeV
            'MU_ONETRACK_PT_MIN': 1000.0, # MeV
            'ONETRACK_IPCHI2_MIN' : 16,   # unitless   
            'V2BODYCHI2_MAX'    : 10,     # unitless
            'NV0_2Body_MAX'     : 2,
            'NV0_3Body_MAX'     : 1,
            'NV0_4Body_MAX'     : 0,
            'MIN_V0_LT'         : '20*ps',
            # bdt cuts
            'BDT_2BODY_MIN'     : 0.4,
            'BDT_3BODY_MIN'     : 0.4,
            'BDT_4BODY_MIN'     : 0.3,
            'BDT_2BODYMU_MIN'   : 0.1,
            'BDT_3BODYMU_MIN'   : 0.1,
            'BDT_4BODYMU_MIN'   : 0.1,
            'BDT_2BODYE_MIN'    : 0.1,
            'BDT_3BODYE_MIN'    : 0.1,
            'BDT_4BODYE_MIN'    : 0.1,
            'BDT_RAD2_MIN'      : 0.1,
            'BDT_RAD2p1_MIN'    : 0.1,
            # bdt param file versions
            'BDT_2BODY_PARAMS'  : 'v1r0',
            'BDT_3BODY_PARAMS'  : 'v1r0',
            'BDT_4BODY_PARAMS'  : 'v1r0',
            # electron filters & cuts
            'PIDE_MIN'          : -2.0},
            # pre and post scales
            'Prescale'   : {'Hlt2Topo2BodySimple' : 0.0,  
                            'Hlt2Topo3BodySimple' : 0.0,
                            'Hlt2Topo4BodySimple' : 0.0,
                            'Hlt2TopoRad2BodyBBDT' : 0.0, 'Hlt2TopoRad2plus1BodyBBDT' : 0.0}, 
            'Postscale'  : {}
            }})

        return d
