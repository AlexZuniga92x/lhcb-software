#// $Id: VeloSim.h,v 1.18 2010-03-04 18:28:35 dhcroft Exp $
#ifndef VELOSIM_H
#define VELOSIM_H 1

// Include files
// from STL
#include <string>

// from Gaudi
#include "GaudiAlg/GaudiAlgorithm.h"
#include "GaudiKernel/RndmGenerators.h"
#include "GaudiKernel/Point3DTypes.h"

// GSL
#include "gsl/gsl_sf_erf.h"
// and FPE gaurd to protect this
#include "Kernel/FPEGuard.h"

// from Velo
#include "Event/MCVeloFE.h"


class DeVelo;
class MCHit;
class VeloChannelID;
class ISiAmpliferResponse;
class ISiDepositedCharge;

/** @class VeloSim VeloSim.h VeloAlgs/MCVeloSim.h
 *
 * Fill MCVeloDigit, based on MCHit
 * In general would contain silicon and Front End Chip Simulation
 * and digitisation.
 * Currently just a fast test version.
 *  @author Chris Parkes, based on code of Oliver Callot; update Tomasz Szumlak
 *  @date   10/01/02
 */

class VeloSim : public GaudiAlgorithm {
public:
  //
  /// Standard constructor
  VeloSim( const std::string& name, ISvcLocator* pSvcLocator );

  virtual ~VeloSim( ); ///< Destructor

  virtual StatusCode initialize();    ///< Algorithm initialization
  virtual StatusCode execute   ();    ///< Algorithm execution
  virtual StatusCode finalize  ();    ///< Algorithm finalise

protected:

private:

/// test if MCVeloFE charge is below m_threshold
  inline bool isLess(LHCb::MCVeloFE* fe) const  {
    return ( (fe->charge()<m_threshold)?true:false );
  }

  ///protected gsl_erf_sf_q against FPE: integral of normal dist from arg->inf.
  inline double safe_gsl_sf_erf_Q(double const &arg) const {
    // remove underflows by ignoring results different from 0 or 1 by < 1e-24
    if(fabs(arg)>10.) return (arg>0. ? 0. : 1.);
    // turn off the inexact FPE (always goes off for this function)
    FPE::Guard reducedFPE(FPE::Guard::mask("Inexact"), true);
    return gsl_sf_erf_Q(arg);
  }

  ///protected poisson distribution against floating point exceptions
  inline int safe_int_poissonDist() {
    // turn off FPEs
    FPE::Guard allFPE(FPE::Guard::mask("AllExcept"), true);
    return int(m_poissonDist());
  }

  /// process requested simulation steps
  StatusCode simulation(LHCb::MCHits * veloHits, double timeOffset);
  
  /// Simulation of charge deposition between entry and exit of MCHit
  void chargeSim(LHCb::MCHits * hits, double timeOffset);

  /// Number of points to deposite charge along the trajectory of the hit
  int simPoints(LHCb::MCHit* hit);

  /// spread the charge over the points to simulate
  void chargePerPoint(LHCb::MCHit* hit,
                      std::vector<double>& Spoints, double timeOffset);

  /// add delta ray charges if required
  void deltaRayCharge(double charge, double tol, std::vector<double>& Spoints);

  /// allow the deposited charge points to diffuse to the strips
  void diffusion(LHCb::MCHit* hit,
                 std::vector<double>& Spoints);

  /// add the charge to a front end channel with MChit link (if not 0)
  void fillFE(LHCb::MCVeloFE* myFE, LHCb::MCHit* hit, double charge);
  
  /// add the charge to a front end channel, no MC link
  inline void fillFE(LHCb::MCVeloFE* myFE, double charge){
    fillFE(myFE, 0, charge);
  }

  /// setup capacitive coupling
  void coupling();

  /// From an originally sorted list, find the strip with the previous key,
  /// or create a new one.
  LHCb::MCVeloFE* findOrInsertPrevStrip(LHCb::MCVeloFEs::iterator FEIt, 
                                        bool& valid, bool& create);

  /// From an originally sorted list, find the strip with the next key,
  /// or create a new one.
  LHCb::MCVeloFE* findOrInsertNextStrip(LHCb::MCVeloFEs::iterator FEIt, 
                                        bool& valid, bool& create);

  void pedestalSim(); ///< add a pedestal value to the FEs
  void noiseSim();    ///< Add noise to the strips 
  /// Added noise of a strip from strip capacitance
  double noiseValue(double noiseSigma);
  /// Noise to add to an otherwise empty strip from strip capacitance
  double noiseValueTail(double noiseSigma, double threshold);
  void CMSim(); ///< Common mode simulation
  void deadStrips(); ///< Simulate random dead strips
  void finalProcess(); ///<remove any MCFEs with charge below abs(threshold)
  StatusCode storeOutputData(); ///< put the data on the TES
  LHCb::MCVeloFE* findOrInsertFE(LHCb::VeloChannelID& stripKey);

  /// gets the effective charge fraction for this hit 
  /// compared to an in time main bunch hit
  double chargeTimeFactor(double TOF, double bunchOffset, double z);

  // check conditions Data Base
  bool checkConditions(LHCb::MCHit* aHit);
  // data members
  std::vector<std::string> m_inputContainers; ///< Name of input containers
  std::vector<double> m_inputTimeOffsets; ///< delta T0 of the input containers
  std::vector<std::string> m_outputContainers;///< Name of output containers
  /// Name of MCHit container to make links to
  std::string m_MCHitContainerToLinkName; 
  /// MCHit container to make links to
  ObjectContainerBase* m_MCHitContainerToLink; 
  //
  DeVelo* m_veloDet; ///< Detector Element
  LHCb::MCVeloFEs* m_FEs; ///< vector of FE output  signals
  LHCb::MCVeloFEs* m_FEs_coupling; ///< vector of coupled FE output signals

  bool m_inhomogeneousCharge; ///< simulate unequal charge along track
  bool m_coupling;            ///< Simulate capactive coupling
  bool m_noiseSim;            ///< Simulate noise in the sensor
  bool m_pedestalSim;         ///< Simulate pedestal variations
  bool m_CMSim;               ///< Simulate common mode effects
  double m_stripInefficiency; ///< Fraction of strips to randomly kill
  // allows resolution to be degraded for robustness studies
  Gaudi::XYZPoint m_movePosition; // used when degrading resolution
  bool m_simNoisePileUp;      ///< velo or pileup for noise simulation
  Rndm::Numbers m_gaussDist;  ///< gaussian random numbers
  Rndm::Numbers m_uniformDist;///< uniform random numbers
  ///< poisson random numbers, mean = av. number of noise strips in a sensor
  Rndm::Numbers m_poissonDist;

  double ran_inv_E2(double Emin, double Emax); ///<random numbers from 1/E^2
  /// Random numbers from a gaussian tail
  double ran_gaussian_tail(const double a, const double sigma);
  /// threshold charge to make VeloFE from strip in ADC counts next to a signal
  double m_thresholdADC;
  /// threshold charge in electrons = m_thresholdADC*m_electronsPerADC
  double m_threshold;
  /// threshold charge to make an isolated VeloFE from strip in ADC counts
  double m_thresholdADCSingle;
  /// threshold charge in electrons = m_thresholdADC*m_electronsPerADC
  double m_thresholdSingle;
  /// probability to add noise tail to otherwise empty strip next to signal
  double m_noiseTailProb;
  /// probability to add noise tail to otherwise empty strip, isolated
  double m_noiseTailProbSingle;
  /// sqrt(2*m_kT/m_biasVoltage) is the expected diffusion width
  double m_kT;
  /// voltage applied to (over)depleted the sensor
  double m_biasVoltage;
  /// Amount of energy required to make an e/hole pair in silicon
  double m_eVPerElectron;
  /// Number of points to distribute charge on per strip
  int m_simulationPointsPerStrip;
  /// Use same charge along strip at each seeding centre
  double m_chargeUniform;
  double m_deltaRayMinEnergy;
  /// fraction next strip charge sharing due to capacitive coupling to simulate
  double m_capacitiveCoupling;
  /// to estimate how many strips to simulate noise in
  double m_averageStripNoise;
  double m_baseDiffuseSigma;    ///< base size of charge diffusion 
  double m_electronsPerADC;     ///< convert noise sources in ADC to electrons
  double m_pulseShapePeakTime;  ///< Time of the peak amplifier response
  double m_offPeakSamplingTime; ///< Time offset of sample to peak time
  double m_pedestalConst;       ///< Simulated pedestal 
  double m_pedestalVariation;   ///< Simulated pedestal variation 
  /// make a VeloFE for every strip (default false)
  bool m_makeNonZeroSuppressedData; 
  double m_noiseScale;          ///< Scale factor to apply to noise from db


  std::string m_SiTimeToolType; ///< normally "SiAmpliferResponse" from STTools
  ISiAmplifierResponse* m_SiTimeTool;

  ISiDepositedCharge* m_depositedCharge;///< Tool calculates accumulated charge
  std::string  m_depChargeToolType;     ///< Name of tool to calculate charge
  bool m_useDepTool; ///< Allows to use GEANT energy directly if false

  unsigned int m_totalFEs; ///< Number of FEs created
  unsigned int m_killedFEsRandom; ///< Number of FEs removed by random kills
  unsigned int m_killedFEsBadStrips; ///< Number of FEs removed by bad strips

  bool m_isDebug; ///< debug level for output
  bool m_isVerbose; ///< debug level for output
  
};
//
#endif // VELOSIM_H
