// $Id: Chi2MuIDTool.h,v 1.1 2009-07-01 18:27:11 polye Exp $
#ifndef CHI2MUIDTOOL_H 
#define CHI2MUIDTOOL_H 1

// Include files
// from Gaudi
#include "GaudiAlg/GaudiTool.h"
#include "ICLTool.h"
#include "IGetArrival.h"
#include "ImuIDTool.h"
#include "IIsMuonCandidateC.h"

#include "MuonDet/MuonBasicGeometry.h"
#include "Event/MuonPID.h"
#include "Event/Track.h"

#include "MuonDet/DeMuonDetector.h"
#include "TrackInterfaces/IMeasurementProvider.h"
#include "TrackInterfaces/ITrackFitter.h"
#include "TrackInterfaces/ITrackExtrapolator.h"

//#include "MuonID/MuonMeasurementProvider.h"
#include "SmartMuonMeasProvider.h"

/** @class Chi2MuIDTool Chi2MuIDTool.h
 *  
 *
 *  @author Jose Angel Hernando Morata
 *  @author Xabier Cid Vidal
 *  @date   2008-07-02
 */
class Chi2MuIDTool : public GaudiTool, public ImuIDTool {
public:
  
  /// Standard constructor
  Chi2MuIDTool( const std::string& type, 
            const std::string& name,
            const IInterface* parent);

  virtual ~Chi2MuIDTool( ); ///< Destructor

  virtual StatusCode initialize();

  
  StatusCode isGoodSeed(const LHCb::Track& seed);
  
  StatusCode muonCandidate(const LHCb::Track& seed, LHCb::Track& muTrack,
                           bool isMuonCandidate,
                           const std::vector<LHCb::LHCbID> ids_init = std::vector<LHCb::LHCbID>());
  
  virtual StatusCode muonQuality(LHCb::Track& muTrack, double& Quality);
  
  StatusCode muonArrival(LHCb::Track& muTrack, double& Arrival);
  
  StatusCode muonDLL(LHCb::Track& muTrack, const double& Quality, double& CLQuality, 
                     const double& Arrival, double& CLArrival, double& DLL);
 
 
  
  
  StatusCode muonID(const LHCb::Track& seed,LHCb::Track& muTrack, 
                    bool isMuonCandidate,
                    double& Quality, double& CLQuality, 
                    double& Arrival, double& CLArrival, double& DLL,
                    const std::vector<LHCb::LHCbID> ids_init = std::vector<LHCb::LHCbID>());


  bool isTrackInsideStation(const int& istation);

  bool isTrackInAcceptance(const LHCb::Track& seed);

  StatusCode makeStates(const LHCb::Track& seed);
  
  void addLHCbIDsToMuTrack(LHCb::Track& muTrack,double mom);
  
  StatusCode search(const LHCb::Track& seed, LHCb::Track& muTrack);

  virtual StatusCode finalize() {return StatusCode::SUCCESS;} //tool finalization

  
protected:
  
  ITrackFitter* m_fitter;
  ITrackExtrapolator* m_extrapolator;
  IMeasurementProvider*  m_measProvider;

  IIsMuonCandidateC* m_IsMuonTool;
  ICLTool* m_CLQuality;
  //ICLTool* m_CLArrival;
  IGetArrival* m_GetArrival;

  DeMuonDetector*  m_mudet;

  double m_nsigmas;
  double m_nsigmasUsed;
  int m_discrval;
  bool m_useBkg;

  LHCb::State* m_mySeedState;

  double m_minmom;
  double m_2hits;
  double m_3hits;

  bool m_arrivalCuts;
  bool m_applyIsMuon;


  std::vector<double> m_MomRangeIsMuon;

  std::vector < double > m_zstations;

  std::vector < int > m_stations_acceptance;

  std::vector < LHCb::State > m_states;

  SmartMuonMeasProvider* m_muonProvider;

};

#endif // MUIDTOOL_H
