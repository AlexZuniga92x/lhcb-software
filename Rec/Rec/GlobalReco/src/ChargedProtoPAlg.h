// $Id: ChargedProtoPAlg.h,v 1.6 2002-11-11 19:27:32 gcorti Exp $
#ifndef CHARGEDPROTOPALG_H 
#define CHARGEDPROTOPALG_H 1

// Include files
// from STL
#include <string>
#include <map>

// from Gaudi
#include "GaudiKernel/Algorithm.h"
// LHCbKernel
#include "Relations/IAssociatorWeighted.h"
// from Event
#include "Event/CaloHypo.h"
#include "Event/TrStoredTrack.h"


/** @class ChargedProtoPAlg ChargedProtoPAlg.h
 *  This algorithms combined information from Tracking,RichPID and
 *  MuonPID to produce charged ProtoParticles
 *
 *  @author Gloria Corti
 *  @date   2002-07-08
 */
class ChargedProtoPAlg : public Algorithm {
public:

  enum TrkRejectType { KeepTrack=0, NoTrack, NoTrackType, Chi2Cut };
  enum ProtoType { TrackProto=0, RichProto, MuonProto, ElectronProto };
  
  /// Standard constructor
  ChargedProtoPAlg( const std::string& name, ISvcLocator* pSvcLocator );

  virtual ~ChargedProtoPAlg( ); ///< Destructor

  virtual StatusCode initialize();    ///< Algorithm initialization
  virtual StatusCode execute   ();    ///< Algorithm execution
  virtual StatusCode finalize  ();    ///< Algorithm finalization

protected:

  /// Check if track satisfy criterias to make a ProtoParticle.
  /// Forward, Match and Upstream tracks with no error flag and
  /// Chi2 < Max value are taken
  int rejectTrack( const TrStoredTrack* track );
  StatusCode addRich( SmartDataPtr<RichPIDs>& richpids, ProtoParticle* proto );
  
private:
  
  std::string m_tracksPath;        ///< Location in TES of input tracks
  std::string m_richPath;          ///< Location in TES of input Rich pids
  std::string m_muonPath;          ///< Location in TES of input Muon pids
  std::string m_electronPath;      ///< Location in TES of input Electron pids
  std::string m_photonMatchName;   ///< Location in TES of calo&track match
  std::string m_electronMatchName; ///< Location in TES of cluster energy match
  std::string m_bremMatchName;     ///< Location in TES of bremsstrahlung match

  std::string m_protoPath;    ///< Location in TES of output ProtoParticles
  
  bool   m_upstream;       ///< Use or not unique upstream tracks
  bool   m_velott;         ///< Use or not unique velo-tt tracks
  double m_trackClassCut;  ///< Fraction of IT clusters to separate Tracks types
  double m_chiSqITracks;   ///< Max Chi2/NoF to make ProtoP from IT Tracks
  double m_chiSqOTracks;   ///< Max Chi2/NoF to make ProtoP from IT Tracks
  
 
  int m_idElectron;           ///< PDG id of e+/-
  int m_idMuon;               ///< PDG id of mu+/mu-
  int m_idPion;               ///< PDG id of pi+/pi-
  int m_idKaon;               ///< PDG id of K+/K-
  int m_idProton;             ///< PDG id of p/p~ 

  typedef IAssociatorWeighted<CaloCluster,TrStoredTrack,float> PhotonMatch;
  typedef IAssociatorWeighted<CaloHypo,TrStoredTrack,float>    ElectronMatch;
  typedef IAssociatorWeighted<CaloHypo,TrStoredTrack,float>    BremMatch;
  typedef const PhotonMatch::InverseType    PhotonTable;
  typedef const ElectronMatch::InverseType  ElectronTable;
  typedef const BremMatch::InverseType      BremTable;
  typedef const PhotonTable::Range   PhotonRange;
  typedef const ElectronTable::Range ElectronRange;
  typedef const BremTable::Range     BremRange;

  PhotonMatch*   m_photonMatch;     ///< Name of photon Match
  ElectronMatch* m_electronMatch;   ///< Name of electron Match
  BremMatch*     m_bremMatch;       ///< Name of brem Match

  typedef std::map< std::string, int, std::less<std::string> > ErrorTable;
  ErrorTable m_errorCount;          ///< Error counters
//    bool m_monitor;                   ///< Fill histograms and special print

};
#endif // CHARGEDPROTOPALG_H
