// $Id: HltMuonRec.h,v 1.3 2008-01-22 09:59:19 hernando Exp $
#ifndef HLTMUON_H 
#define HLTMUON_H 1

// Include files
// from Gaudi
#include "GaudiAlg/GaudiAlgorithm.h"
#include "GaudiAlg/ISequencerTimerTool.h"

#include "MuonDet/IMuonPosTool.h"
#include "MuonDet/MuonBasicGeometry.h"
#include "MuonDet/DeMuonRegion.h"
#include "MuonDet/DeMuonDetector.h"
#include "Kernel/IMuonLayout.h"
#include "Event/Track.h"
                              
#include "HltMuonStationRec.h"
#include "HltMuonTrack.h"

#include "HltBase/IMuonSeedTool.h"

/** @class HltMuonRec HltMuonRec.h
 *  
 *
 *  @author Alessia Satta
 *  @date   2004-10-06
 */
class HltMuonRec : public GaudiAlgorithm {
public: 
  /// Standard constructor
  HltMuonRec( const std::string& name, ISvcLocator* pSvcLocator );

  virtual ~HltMuonRec( ); ///< Destructor

  virtual StatusCode initialize();    ///< Algorithm initialization
  virtual StatusCode execute   ();    ///< Algorithm execution
  virtual StatusCode finalize  ();    ///< Algorithm finalization
  StatusCode decodeBuffer();
  StatusCode createCoords(int station);
  StatusCode crossStrips(unsigned int station, unsigned int region) ;
  
  //  unsigned int readoutType(int partition, MuonLayout lay);
  LHCb::MuonTileID intercept(const LHCb::MuonTileID& stripX, 
                             const LHCb::MuonTileID& stripY) ;
  StatusCode muonSearch();
  StatusCode detectClone();    
  StatusCode strongCloneKiller();

  void insertMatch(int muon,LHCb::Track* track,float distance);
	StatusCode printOut();  
  StatusCode initializePadDecoding();

  std::vector<LHCb::MuonTileID> DoPad(std::vector<LHCb::MuonTileID>
                                      digit,MuonTSMap* TS);
  StatusCode getPads(int station);
  StatusCode createCoordsFromLC(int station);
  StatusCode initializeLogChanDecoding();
protected:
private:
  //  IMuonTileXYZTool  *m_iTileTool;
  //  IMuonGeometryTool *m_iGeomTool;  
  //  IMuonGetInfoTool  *m_iGetInfo;
  DeMuonDetector* m_muonDetector;
  IMuonPosTool      *m_iPosTool;
  //ITrackSelector* m_trackSelector; // tool to accept a track
  
  unsigned int m_nStation;
  unsigned int m_nRegion;
  std::vector<std::string> m_stationNames;
  int m_totL1Board;

  std::vector<HltMuonStationRec> m_station;
  std::vector< std::vector<LHCb::MuonTileID> > m_L1Map;
  unsigned int m_L1BoardStart;
  unsigned int m_L1BoardStop;

  unsigned int m_ODENumberInTell1[12];
  std::vector<unsigned int> m_ODEInTell1[12];
  std::vector<LHCb::MuonTileID> m_mapTileInODE[180];



  int m_stationL1Start[5];
  int m_stationL1Stop[5];
// for pad decoding
  unsigned int m_M1Tell1;
  std::string getBasePath(int station);
  std::string basePath[5] ;
  std::vector<LHCb::MuonTileID> m_mapPad[20];



  ISequencerTimerTool* m_timer;
  int m_timeLoad;
  int m_timePad;
  int m_timeMuon;              
  int m_timeTag;
  //  int m_timeAsso;
  int m_timeMuonStore;

  std::vector<std::pair<LHCb::MuonTileID,bool> > twelfthX[12];
  std::vector<std::pair<LHCb::MuonTileID,bool> > twelfthY[12];
  std::vector<HltMuonTrack> m_muonTracks;
  unsigned int m_maxMuonFound;  
  bool m_measureTime;  
  bool m_cloneKiller;
//  bool m_storeTracks;
  bool m_padM5;
  bool m_padM4;
  bool m_padM3;
  bool m_padM2;
  bool m_decodingFromCoord;
  std::string m_outputMuonTracksName;

  // counters
  int m_countEvents;
  int m_countMuCandidates;

protected:

  bool m_doPrepareMuonSeg;
  IMuonSeedTool* m_prepareMuonSeed;  
};
#endif // HLTMUON_H
