
## @package TrackCreator
#  Track creator options for RICH Reconstruction
#  @author Chris Jones  (Christopher.Rob.Jones@cern.ch)
#  @date   15/08/2008

__version__ = "$Id: TrackCreator.py,v 1.11 2009-12-11 13:43:08 cattanem Exp $"
__author__  = "Chris Jones <Christopher.Rob.Jones@cern.ch>"

from RichKernel.Configuration  import *
from GaudiKernel.SystemOfUnits import mm, GeV
from SegmentCreator import *

# ----------------------------------------------------------------------------------

## @class RichRecSysConf
#  Configurable for RICH reconstruction track creator
#  @author Chris Jones  (Christopher.Rob.Jones@cern.ch)
#  @date   15/08/2008
class RichTrackCreatorConfig(RichConfigurableUser):

       ## Possible used Configurables
    __used_configurables__ = [ (RichSegmentCreatorConf,None) ]

    # Steering options
    __slots__ = {
        "Context":       "Offline"  # The context within which to run
       ,"Radiators": [True,True,True] # The radiators to use (Aerogel/Rich1Gas/Rich2Gas)
       ,"SpecialData"    : []   # Various special data processing options. See KnownSpecialData in RecSys for all options
       ,"InputTracksLocation" : "" # The input location for tracks
       ,"OutputLevel"    : INFO    # The output level to set all algorithms and tools to use
       ,"TrackTypes"     : [ "Forward","Match","Seed","VeloTT","KsTrack" ]
        # CRJ : Defaults pre-data cuts "CloneDistCut" : [-1e10,1e300] "Likelihood"   : [-100,1e300]
       ,"TrackCuts"      : None
       ,"MaxInputTracks" : None
       ,"MaxUsedTracks"  : None
        }

    ## Initialize 
    def initialize(self):
        # default values
        self.setRichDefaults ( "MaxInputTracks", { "Offline" : 99999, 
                                                   "HLT"     : 99999 } )
        self.setRichDefaults ( "MaxUsedTracks",  { "Offline" : 500, 
                                                   "HLT"     : 400 } )
        self.setRichDefault ( "TrackCuts", "Offline",
                              { "Forward" : { "Chi2Cut" : [0,10], "PCut" : [0,9999999] },
                                "Match"   : { "Chi2Cut" : [0,10], "PCut" : [0,9999999] },
                                "Seed"    : { "Chi2Cut" : [0,10], "PCut" : [1,9999999] },
                                "VeloTT"  : { "Chi2Cut" : [0,10], "PCut" : [1,9999999] },
                                "KsTrack" : { "Chi2Cut" : [0,10], "PCut" : [1,9999999] } } )
        self.setRichDefault ( "TrackCuts", "HLT",
                              { "Forward" : { "Chi2Cut" : [0,16], "PCut" : [1,9999999] } } )

    ## @brief Set OutputLevel 
    def setOutputLevel(self,conponent):
        if self.isPropertySet("OutputLevel") :
            conponent.OutputLevel = self.getProp("OutputLevel")

    ## @brief Apply the configuration
    #
    def applyConf(self):

        # segments
        segConf = self.getRichCU(RichSegmentCreatorConf)
        self.setOtherProp(segConf,"Context")

        # Configure the tracking tools
        nickname = "RichTrackCreator"
        
        # Track creator
        trackCr = self.richTools().trackCreator(nickname)

        # Track cuts
        trackCr.MaxInputTracks = self.getProp("MaxInputTracks")
        trackCr.MaxUsedTracks  = self.getProp("MaxUsedTracks")

        # Input tracks
        if not self.isPropertySet("InputTracksLocation"):
            raise RuntimeError("ERROR : Input Track Location not set" )
        trackCr.TracksLocation = self.getProp("InputTracksLocation")
            
        # Track selector
        trselname = "TrackSelector"
        trackCr.addTool( self.richTools().trackSelector(trselname), name=trselname )
        trackCr.TrackSelector.TrackAlgs = self.getProp("TrackTypes")
        import TrackSelectorTools
        TrackSelectorTools.configureTrackSelectorCuts(trackCr.TrackSelector,self.getProp("TrackCuts"))
           
        # Segment maker
        if trackCr.getType() != "Rich::Rec::DelegatedTrackCreatorFromRecoTracks" :
            trSegNickName = "TrSegMaker"
            trackCr.TrackSegmentTool = trSegNickName
            trSeg = self.richTools().trSegMaker()
            trSeg.UseRadiators = self.getProp("Radiators")
            trackCr.addTool( trSeg, trSegNickName )

        # OutputLevels
        self.setOutputLevel(trackCr)
        self.setOutputLevel(self.richTools().expectedTrackSignal())

        # Printout
        self.printInfo(segConf)
