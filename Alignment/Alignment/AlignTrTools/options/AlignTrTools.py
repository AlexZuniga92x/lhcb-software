from Configurables import ( TAConfig, Derivatives, ATrackSelector, WriteAlignmentConditions )

TAConfig().MillepedeTool = "Centipede"
TAConfig().DerivativeTool = "Derivatives"
TAConfig().MeasurementProvider = "MuonMeasurementProvider"
TAConfig().nTrackModelParameters = 4
TAConfig().Degrees_of_Freedom = []
TAConfig().Constraint_Equations = []
TAConfig().Chi_sq_factor = 10.0
TAConfig().chi2_scale = 10000.0
TAConfig().n_f_stdev = 10.0
TAConfig().residual_cut = 20.0
TAConfig().initial_residual_cut = 10.0
TAConfig().n_l_stdev = 10
TAConfig().Velo_left_modules = []
TAConfig().Velo_right_modules = []
TAConfig().Fix_alignable_objects = True
TAConfig().Constrain_IT = []
TAConfig().Constrain_OT = []
TAConfig().Constrain_VeLo = []
TAConfig().Constrain_Muon = []
TAConfig().Constrain_TT = []

TAConfig().OT_system = False
TAConfig().OT_station = False
TAConfig().OT_layer = False
TAConfig().OT_quadrant = False
TAConfig().OT_module = False

TAConfig().IT_system = False
TAConfig().IT_station = False
TAConfig().IT_box = False
TAConfig().IT_layer = False
TAConfig().IT_ladder = False

TAConfig().MUON_system = False
TAConfig().MUON_stations = False
TAConfig().MUON_chambers = False

TAConfig().TT_system = False
TAConfig().TT_station = False
TAConfig().TT_layer = False

TAConfig().IT_alignment_condition_location = ""
TAConfig().TT_alignment_condition_location = ""
TAConfig().OT_alignment_condition_location = ""

WriteAlignmentConditions().topElement = ""
WriteAlignmentConditions().footer = "</DDDB>"
WriteAlignmentConditions().startTag = "<condition"
WriteAlignmentConditions().outputFile = "alignment.xml"
WriteAlignmentConditions().depths = []
WriteAlignmentConditions().precision = 16

ATrackSelector().MinPCut = 0.0
ATrackSelector().MinPtCut = 0.0
ATrackSelector().MinEnergyCut = 0.0
ATrackSelector().MinChi2Cut = 0.0
ATrackSelector().Charge = 0
ATrackSelector().MaxPCut = []
ATrackSelector().MaxPtCut = []
ATrackSelector().MaxChi2Cut = []
ATrackSelector().MaxHitCut = []

