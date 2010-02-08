// $Id: PatAddTTCoord.h,v 1.4 2010-02-08 16:45:11 decianm Exp $
#ifndef PATADDTTCOORD_H
#define PATADDTTCOORD_H 1

// Include files
// from Gaudi
#include "GaudiAlg/GaudiTool.h"

#include "Event/Track.h"
#include "Event/State.h"
#include "TrackInterfaces/IAddTTClusterTool.h"            // Interface
#include "PatKernel/PatTTHit.h"
#include "Kernel/ILHCbMagnetSvc.h"
#include "TfKernel/TTStationHitManager.h"



  /** @class PatAddTTCoord PatAddTTCoord.h
   *
   * \brief  Adds TT clusters to tracks 
   *
   * Parameters:
   * - ZTTField: Z-Position of the kink for the state extrapolation
   * - ZMSPoint: Z-Position of the multiple scattering point
   * - TTParam: Parameter of the slope of the state extrapolation
   * - MaxChi2Tol: Offset of the chi2 cut
   * - MaxChi2Slope: Slope of the chi2 cut
   * - MaxChi2POffset: Momentum offest of the chi2 cut
   * - YTolSlope: Offest of the y-tolerance cut
   * - XTol: Offest of the x-window cut
   * - XTolSlope: Slope of the x-window cut
   * - MaxProjTol: Offest of the projection cut
   * - MaxProjSlope: Slope of the projection cut
   * - ZTTProj: Z-Position which state has to be closest to
   *  
   *  @author Olivier Callot
   *  @author Michel De Cian (added method and code restructured)
   *  @date   2010-02-04 
   *  
   */

class PatAddTTCoord : public GaudiTool, virtual public IAddTTClusterTool {
public:
    /// Standard constructor
  PatAddTTCoord( const std::string& type,
                 const std::string& name,
                 const IInterface* parent);

  virtual ~PatAddTTCoord( ); ///< Destructor
  
  virtual StatusCode initialize();

  /// Add TT clusters to matched tracks
  virtual StatusCode addTTClusters( LHCb::Track& track);
  virtual StatusCode returnTTClusters( LHCb::State& state, PatTTHits& ttHits, double& finalChi2, double p = 0 );

  virtual StatusCode addTTClusters( LHCb::Track& /*track*/, 
                                    std::vector<LHCb::STCluster*>& /*ttClusters*/,
                                    std::vector<double>& /*ttChi2s*/ ) { return StatusCode::SUCCESS;};
  

  virtual double distanceToStrip( const LHCb::Track& /*track*/, 
                                  const LHCb::STCluster& /*ttCluster*/ ) {return 0.;};
  
  
    
 

private:

  void selectHits(PatTTHits& selected, const LHCb::State& state, const double p);
  void calculateChi2(PatTTHits& goodTT,  double& chi2, const double& bestChi2, double& finalDist, const double& p );
  void printInfo(const PatTTHits& goodTT, double dist, double chi2, const LHCb::State& state);
  StatusCode debugEnd( PatTTHits goodTT , const double slope, 
                       const double offsetY, const double offset, const LHCb::State state, const double chi2);



  Tf::TTStationHitManager<PatTTHit> * m_ttHitManager;
  double m_ttParam;
  double m_zTTField;
  double m_zTTProj;
  double m_zMSPoint;
  
  double m_maxChi2Tol;
  double m_maxChi2Slope;
  double m_maxChi2POffset;
  double m_yTolSlope;
  double m_xTol;
  double m_xTolSlope;
  double m_maxProjTol;
  double m_maxProjSlope;
  std::string m_extrapolatorName;
  
  ILHCbMagnetSvc*     m_magFieldSvc; 


};

#endif // PATADDTTCOORD_H
