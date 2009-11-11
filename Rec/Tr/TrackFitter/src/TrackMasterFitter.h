// $Id: TrackMasterFitter.h,v 1.27 2009-11-11 21:12:52 wouter Exp $
#ifndef TRACKFITTER_TRACKMASTERFITTER_H 
#define TRACKFITTER_TRACKMASTERFITTER_H 1

// Include files
// -------------
// from Gaudi
#include "GaudiAlg/GaudiTool.h"
#include "GaudiKernel/ToolHandle.h"

// interface base class
#include "TrackInterfaces/ITrackFitter.h"
#include "TrackInterfaces/ITrackProjectorSelector.h"

// Forward declarations
class ITrackManipulator ;
class IMaterialLocator ;
class ITrackExtrapolator ;
class IMeasurementProvider ;

namespace LHCb {
  class Track ;
  class FitNode ;
  class State ;
}

/** @class TrackMasterFitter TrackMasterFitter.h
 *  
 *
 *  @author Jose Angel Hernando Morata, Eduardo Rodrigues
 *  @date   2005-04-15
 *  reusing the previous code
 *  @author Rutger van der Eijk  07-04-1999
 *  @author Matthew Needham 
 */

class TrackMasterFitter : public GaudiTool,
                          virtual public ITrackFitter {
public: 
  /// Standard constructor
  TrackMasterFitter( const std::string& type, 
                     const std::string& name,
                     const IInterface* parent );

  /// Destructor
  virtual ~TrackMasterFitter( );

  StatusCode initialize();
  StatusCode finalize();

  //! fit a track 
  StatusCode fit( LHCb::Track& track, LHCb::ParticleID pid = LHCb::ParticleID(211)  ) ;

private:

  //! initialize reference states for initial trajectory
  StatusCode initializeRefStates(LHCb::Track& track, LHCb::ParticleID pid ) const ;

  //! determine track state at various z positions
  StatusCode determineStates( LHCb::Track& track ) const;

  //! remove outliers from the node vector
  bool outlierRemoved( LHCb::Track& track ) const;

  //! update the reference vector for each measurement before next iteration
  StatusCode updateRefVectors( LHCb::Track& track ) const;

  //! projectReference state
  StatusCode projectReference( LHCb::Track& track ) const;

  //! determine the z-position of the closest approach to the beam line
  //! by linear extrapolation.
  double closestToBeamLine( const LHCb::State& state ) const;

  //! Retrieve the number of nodes with a measurement
  unsigned int nNodesWithMeasurement( const LHCb::Track& track ) const;

  //! Create the nodes from the measurements
  StatusCode makeNodes( LHCb::Track& track, LHCb::ParticleID pid ) const;

  //! Add info from fitter as extrainfo to track
  void fillExtraInfo( LHCb::Track& track ) const ;

  //! Update material corrections stored in nodes
  StatusCode updateMaterialCorrections( LHCb::Track& track, LHCb::ParticleID pid ) const ;

  //! Update transport matrices stored in nodes
  StatusCode updateTransport( LHCb::Track& track ) const ;

  ITrackExtrapolator* extrapolator( LHCb::Track::Types tracktype ) const {
    return tracktype == (LHCb::Track::Velo || LHCb::Track::VeloR) ? m_veloExtrapolator : m_extrapolator ;
  }
  
private:

  ITrackExtrapolator* m_extrapolator;     ///< extrapolator
  ITrackExtrapolator* m_veloExtrapolator; ///< extrapolator for Velo-only tracks
  ITrackKalmanFilter* m_trackNodeFitter;    ///< delegate to actual track fitter (which fits from nodes)
  IMeasurementProvider* m_measProvider;
  IMaterialLocator*     m_materialLocator ;
  ToolHandle<ITrackProjectorSelector> m_projectorSelector ;
  std::string           m_materialLocatorName ;

private:

  // job options
  std::string m_extrapolatorName;   ///< name of the extrapolator in Gaudi
  std::string m_veloExtrapolatorName; ///< name of the velo-only extrapolator 
  bool m_upstream;                  ///< switch between upstream/downstream fit
  bool m_addDefaultRefNodes  ;      ///< add default reference nodes
  bool m_stateAtBeamLine;           ///< add state closest to the beam-line?
  int m_numFitIter;                 ///< number of fit iterations to perform
  double m_chi2Outliers;            ///< chi2 of outliers to be removed
  int m_numOutlierIter;             ///< max number of outliers to be removed
  bool m_useSeedStateErrors;        ///< use errors of the seed state
  
  double m_errorX ;                 ///< Seed error on x
  double m_errorY ;                 ///< Seed error on y
  double m_errorTx ;                ///< Seed error on slope x
  double m_errorTy ;                ///< Seed error on slope y
  std::vector<double> m_errorQoP ;  ///< Seed error on QoP

  bool m_makeNodes;
  bool m_makeMeasurements;
  bool m_updateTransport ;            ///< Update the transport matrices between iterations
  double m_minMomentumForELossCorr ;  ///< Minimum momentum used in correction for energy loss
  bool m_applyMaterialCorrections ;   ///< Apply material corrections
  bool m_applyEnergyLossCorrections ; ///< Apply material corrections
  double m_maxDeltaChi2Converged ;    ///< Maximum change in chisquare for converged fit

  double m_scatteringPt ;           ///< transverse momentum used for scattering if track has no good momentum estimate
  double m_maxMomentumForScattering ; ///< Maximum momentum used for scattering
  size_t m_minNumVeloRHits   ; ///< Minimum number of VeloR hits
  size_t m_minNumVeloPhiHits ; ///< Minimum number of VeloPhi hits
  size_t m_minNumTTHits      ; ///< Minimum number of TT hits
  size_t m_minNumTHits       ; ///< Minimum number of T hits
  size_t m_minNumMuonHits    ; ///< Minimum number of Muon hits

  //! helper to print a failure comment
  StatusCode failure( const std::string& comment ) const;

  bool m_debugLevel;

};
#endif // TRACKFITTER_TRACKKALMANFILTER_H
