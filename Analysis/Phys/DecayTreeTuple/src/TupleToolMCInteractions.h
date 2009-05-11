// $Id: TupleToolMCInteractions.h,v 1.4 2009-05-11 14:45:55 rlambert Exp $
#ifndef TUPLETOOLGENERATION_H
#define TUPLETOOLGENERATION_H 1

// Include files
// from Gaudi
#include "GaudiAlg/GaudiTool.h"
#include "GaudiKernel/SystemOfUnits.h"
#include "Kernel/IEventTupleTool.h"            // Interface

class ITupleTool;

/** @class TupleToolMCInteractions TupleToolMCInteractions.h rlambert/TupleToolMCInteractions.h
 *
 * \brief Number of interactions. This TupleTool will allow the calculation of a scaling factor
 *          as an event-by-event weight. When applied to events it will approximate a different mean number of
 *          interactions per event.
 *        The starting mean number of interactions will by default be taken from the GenHeader. 
 *        If this is not available it will set to the DC06 default
 *        The mean can be overwritten by setting the option "Mean" as below
 *
 * Tuple columns:
 * 
 *  EVT_Int_I    unsigned int   number of interactions
 *  EVT_Int_Mean double         mean number of interactions
 *  EVT_Int_Prob double         probability of generating this number of interactions
 *  
 * If the scaling option (AdjustMean) is set
 *  EVT_Int_AdjustMean    double         mean number of interactions to weight to
 *  EVT_Int_AdjustProb    double         probability of this event in the adjusted case
 *  EVT_Int_AdjustWeight  double         weight to apply to change from mean to adjust mean
 *
 * If the details option (FillDetails) is set
 *  EVT_Int_NormalisedAt   unsigned int   chosen place to normalise the weight
 *  EVT_Int_NormalisedTo   double         weights normalised to this value
 *  EVT_Int_nMCI           unsigned int   number of collisions from the GenHeader
 *  EVT_Int_nMCPV          unsigned int   number of collisions from the MCHeader
 *  EVT_Int_nRecPV         unsigned int   number of reconstructed PVs
 *
 * Options:
 *  Mean        double  overwrite the mean value of interactions for this event
 *                       -default is described above
 *  AdjustMean  double  calculate a scaling factor to approximate this number of interactions per event
 *                       -by default this is zero, and no scaling information is filled
 *  NormaliseAt int     normalise the scaling factor such that it is equal to 1 at this value of I
 *                       -by default this is set to zero, and no normalisation is performed
 *  Prefix        string  prefix to give to the variables, in case you want to use two copies of this tool
 *                       -by default this is "EVT_Int"
 *  UseRecPV      bool  use the #of reconstructed PVs, rather than the MC Collisions
 *                       -by default this is false
 *  FillDetails   bool  fill extra information on MCPV, MC Collisions and Reconstructed PV
 *                       -by default this is false, fine for most purposes
 *
 * \sa DecayTreeTuple
 *  @author P. Koppenburg
 *  @date   2008-07-01
 */
class TupleToolMCInteractions : public GaudiTool, virtual public IEventTupleTool {
public:
  /// Standard constructor
  TupleToolMCInteractions( const std::string& type,
		      const std::string& name,
		      const IInterface* parent);

  virtual ~TupleToolMCInteractions( ){}; ///< Destructor

  StatusCode fill( Tuples::Tuple& );
  virtual StatusCode initialize();

private :

  double m_mean;       ///<mean number of interactions per event. set by the option Mean. Default of zero will get the result from the GenHeader
  double m_adjustmean; ///<mean number of interactions to weight to. set by the option AdjustMean. Default of zero will not calculate the adjustment.
  unsigned int m_normaliseAt; ///<where to place the normalisation such that the weight is one at this value. set by the option NormaliseAt. Default of zero will not calculate specific normalisation, but use the correct weighting from te poisson.
  std::string m_prefix; ///<mean number of interactions to weight to. set by the option AdjustMean. Default of zero will not calculate the adjustment.
  bool m_useRecPV; ///<use the #of reconstructed PVs, rather than the MC Collisions. set by the option UseRecPV.
  bool m_fillDetails; ///<fill extra information on MCPV, MC Collisions and Reconstructed PVs. set by the option FillDetails.
 
  ///calculate the probability of n interactions given that the mean is mu
  double poisson(const double mu, const unsigned int n);
  ///calculate the ratio of the probabilities for n interactions with the two means mu1, mu2
  double weight(const double mu1, const double mu2, const unsigned int n);
  ///calculate the factorial of n
  double factorial(const unsigned int n);
  ///calculate a^x
  double power(const double a, const unsigned int x);

};
#endif // TUPLETOOLGENERATION_H
