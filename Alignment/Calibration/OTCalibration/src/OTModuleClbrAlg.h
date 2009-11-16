#ifndef __OTModuleClbrAlg_h 
#define __OTModuleClbrAlg_h 1

#include "GaudiAlg/GaudiHistoAlg.h"
#include "GaudiKernel/AlgFactory.h"

#include "GaudiKernel/ToolHandle.h"
#include "TrackInterfaces/ITrackProjector.h"
#include "OTDet/DeOTDetector.h"

#include "Event/OTMeasurement.h"
#include "Event/FitNode.h"
#include "Event/State.h"

#include <string>

/** @class OTModuleClbrAlg OTModuleClbrAlg.h
 *
 *  This class implements algorithms for module T0's and TR-relation
 *  calibration.
 *
 *  The granularity is per module.
 *
 *  @author Alexandr Kozlinskiy
 *  @date   2009-10-20
 */
class OTModuleClbrAlg: public GaudiHistoAlg
{
private:
  AIDA::IHistogram1D* histModuleDriftTime[3][4][4][9];
  AIDA::IHistogram1D* histModuleDriftTimeResidual[3][4][4][9];
  AIDA::IHistogram2D* histModuleDriftTimeVsDist[3][4][4][9];

  AIDA::IHistogram2D* histDriftTimeVsDist;

  AIDA::IHistogram1D* histModuleDT0VsModuleN;
  AIDA::IHistogram1D* histModuleT0VsModuleN;

  AIDA::IHistogram1D* histModuleDT0;
  AIDA::IHistogram1D* histModuleT0;
private:
  /**
   * Options for drift time histograms (default t = [-30, 70]/200).
   */
  std::vector<double> histDriftTimeOpts;
  /**
   * Options for drift time residual histograms (default t = [-50, 50]/200).
   */
  std::vector<double> histDriftTimeResidualOpts;
  /**
   * Options for drift time VS distance histograms (default r = [-5, 5]/200 and t = [-30, 70]/200).
   */
  std::vector<double> histDriftTimeVsDistOpts;
  /**
   * Options for module dT0's histogram (default t = [-10, 20]/120).
   */
  std::vector<double> histModuleDT0Opts;
  /**
   * Options for module T0's histogram (default t = [-10, 20]/120).
   */
  std::vector<double> histModuleT0Opts;

  /**
   * Plot drift time histograms per module (default false).
   */
  bool plotModuleDriftTime;
  /**
   * Plot drift time VS distance histograms per module (default false).
   */
  bool plotModuleDriftTimeVsDist;

  double t0s[3][4][4][9];
  double trA, trB, trC;
  double trSigmaA, trSigmaB, trSigmaC;

  DeOTDetector* detector;

  std::string trackLocation;

  ToolHandle<ITrackProjector> projector;

  /**
   * Max track Chi^2 (default 16).
   */
  double maxTrackChi2;
  /**
   * Max track unbiased Chi^2 (default 4).
   */
  double maxUnbiasedChi2;

  /**
   * Use outliers (default false).
   */
  bool useOutliers;

  /**
   * Write condition XML files (default true).
   */
  bool writeXMLs;
  /**
   * Read condition XML files (default true).
   */
  bool readXMLs;

  /**
   * Use Minuit to fit TR-relation (default true).
   */
  bool useMinuit;

  /**
   * Station names: T1, T2, T3.
   */
  std::vector<std::string> stationNames;
  /**
   * Layer names: X1, U, V, X2
   */
  std::vector<std::string> layerNames;
  /**
   * Quarter names: Q0, Q1, Q2, Q3.
   */
  std::vector<std::string> quarterNames;
  /**
   * Module names: M1, M2, ... , M9.
   */
  std::vector<std::string> moduleNames;
public:
  OTModuleClbrAlg(const std::string& name, ISvcLocator* pSvcLocator);

  StatusCode initialize();
  StatusCode execute();
  StatusCode finalize();
private:
  void fillHists(const LHCb::OTMeasurement& measurement, const LHCb::FitNode& node, const LHCb::State& state);

  /**
   * Fit module T0's using drift time residuals histograms.
   */
  StatusCode fitT0s();

  /**
   * Simple TR-relation fit.
   */
  StatusCode fitTR();

  /**
   * Fit TR-relation using Minuit.
   */
  StatusCode fitTRMinuit();

  /**
   * Write condition XMLs.
   */
  StatusCode writeCondXMLs();

  /**
   * Read condition XMLs.
   */
  StatusCode readCondXMLs();
};

DECLARE_ALGORITHM_FACTORY(OTModuleClbrAlg);

#endif // __OTModuleClbrAlg_h
