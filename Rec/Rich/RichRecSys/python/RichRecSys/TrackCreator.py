
## @package TrackCreator
#  Track creator options for RICH Reconstruction
#  @author Chris Jones  (Christopher.Rob.Jones@cern.ch)
#  @date   15/08/2008

__version__ = "$Id: TrackCreator.py,v 1.6 2009-04-07 16:17:17 jonrob Exp $"
__author__  = "Chris Jones <Christopher.Rob.Jones@cern.ch>"

from RichKernel.Configuration  import *
from GaudiKernel.SystemOfUnits import mm, GeV
from RichRecSys.SegmentCreator import *

# ----------------------------------------------------------------------------------

## @class RichRecSysConf
#  Configurable for RICH reconstruction track creator
#  @author Chris Jones  (Christopher.Rob.Jones@cern.ch)
#  @date   15/08/2008
class RichTrackCreatorConfig(RichConfigurableUser):

    # Steering options
    __slots__ = {
        "Context":       "Offline"  # The context within which to run
       ,"Radiators": [True,True,True] # The radiators to use (Aerogel/Rich1Gas/Rich2Gas)
       ,"SpecialData"  : []      # Various special data processing options. See KnownSpecialData in RecSys for all options
       ,"InputTracksLocation" : "" # The input location for tracks
        }

    ## @brief Apply the configuration
    #
    def applyConf(self):

        # segments
        segConf = RichSegmentCreatorConf()
        self.setOtherProps(segConf,["Context"])
        segConf.applyConf()
        del segConf

        # Configure the tracking tools
        nickname = "RichTrackCreator"
        
        # Track creator
        trackCr = RichTools().trackCreator(nickname)

        # Input tracks
        intracks = self.getProp("InputTracksLocation")
        if intracks != "" : trackCr.TracksLocation = intracks
            
        # Track selector
        trselname = "TrackSelector"
        trackCr.addTool( RichTools().trackSelector(trselname), name=trselname )
            
        # Segment maker
        if trackCr.getType() != "Rich::Rec::DelegatedTrackCreatorFromRecoTracks" :
            trSegNickName = "TrSegMaker"
            trackCr.TrackSegmentTool = trSegNickName
            trSeg = RichTools().trSegMaker()
            trSeg.UseRadiators = self.getProp("Radiators")
            trackCr.addTool( trSeg, trSegNickName )

# ----------------------------------------------------------------------------------

## @class TrackCreatorFromCheatedRecoTracks
#  To Be Removed
#  @author Chris Jones  (Christopher.Rob.Jones@cern.ch)
#  @date   15/08/2008
class TrackCreatorFromCheatedRecoTracks(RichConfigurableUser):
    
    __slots__ = {
        "Context" : "Offline",
        "ToolsByTrackType" : [ "Forward/RichTrackCreatorMC",
                               "Match/RichTrackCreatorMC",
                               "VeloTT/RichTrackCreatorMC",
                               "Seed/RichTrackCreatorMC",
                               "KsTrack/RichTrackCreatorMC",
                               "Velo/RichTrackCreatorMC" ]
        }

    def createInstance(self,parent,nickname):

        from Configurables import Rich__Rec__DelegatedTrackCreatorFromRecoTracks
        
        context = parent.getProp("Context")
        self.setProp("Context",context)
        
        type    = "Rich::Rec::DelegatedTrackCreatorFromRecoTracks"
        self.toolRegistry().Tools += [ type+"/"+nickname ]
        trackCr = Rich__Rec__DelegatedTrackCreatorFromRecoTracks("ToolSvc."+context+"_"+nickname)
        trackCr.ToolsByTrackType = self.getProp("ToolsByTrackType")

        recomakername = "RichTrackCreatorReco"
        trSegNickName = "RichDetTrSegMakerReco"
        recomaker = Rich__Rec__TrackCreatorFromRecoTracks(recomakername)
        self.toolRegistry().Tools += [ recomaker ]
        recomaker.TrackSegmentTool = trSegNickName;
        trackCr.addTool( recomaker, name=recomakername )
        trSegReco = parent.defaultTrSegMaker(trSegNickName,"Detailed")
        recomaker.addTool( trSegReco, name=trSegNickName )

        recomakername = "RichTrackCreatorMC"
        trSegNickName = "RichDetTrSegMakerMC"
        recomaker = Rich__Rec__TrackCreatorFromRecoTracks(recomakername)
        self.toolRegistry().Tools += [ recomaker ]
        recomaker.TrackSegmentTool = trSegNickName;
        trackCr.addTool( recomaker, name=recomakername )
        trSegReco = parent.defaultTrSegMaker(trSegNickName,"MCRichTracks")
        recomaker.addTool( trSegReco, name=trSegNickName )

        return trackCr
            
