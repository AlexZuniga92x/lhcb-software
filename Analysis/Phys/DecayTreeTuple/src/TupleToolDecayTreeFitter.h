/**@class TupleToolDecayTreeFitter 
    * -  History : this tool was written during the study of the b->JpsiX lifetimes. It was not very generic.
    *    This re-implmentation is hopefully more generic, although it requires several instances of the tool
    *    to achive the same result.
    * -  Advice  : Most of the output of this code could be saved via the LoKi style lines.    

    * -  Usage   : One can plug this Tool in DecayTreeTuple. It is advised to give it to a branch and not to try to run 
    *    it on all the tree. To get full functionality use several instances. 
    *    The instance name will be used as prefix. See example.
    *  - Configuration : 
 @code   
	constrainToOriginVertex = False          # save a fit to compute the lifetime. 
	particlesToConstrain = ["J/psi(1S)"]     # chose the particles to mass to constrain. 
  ExtraName = ""                           # Additional Prefix (will replace instance name) (inherited from TupleToolBase)
  Verbose = False                          # Fill Lifetime for daughters. (inherited from TupleToolBase) 
@endcode 
    * 
    * -  Outputs: for each fit is saved: 
         - The status, the chi2, the number of degres of freedom of the fit. 
	       - The fitted mass, mass error for example : Lambda_b0_massConAllDaughters_M ...
	       - The fitted lifetime, and error : Lambda_b0_taufit_ctau (ctau is given in mm).
    * 
    * -  Example:
@code
decay = "B0 -> (^J/psi(1S) -> ^mu+ ^mu-) (^KS0 -> ^pi+ ^pi-)"
bh = 'B0'
tuple.Decay = decay
tuple = DecayTreeTuple("MyTuple")
tuple.ToolList +=  ["TupleToolGeometry",  "TupleToolKinematic", "TupleToolPrimaries", "TupleToolEventInfo" ]
tuple.Branches = { "B"  : "["+bh+"]cc : "+decay.replace("^","") }
from Configurables import TupleToolDecayTreeFitter
tuple.B.ToolList +=  [ "TupleToolDecayTreeFitter/Fit",            # just a refit
                       "TupleToolDecayTreeFitter/MassFit",        # fit with J/psi mass constraint
                       "TupleToolDecayTreeFitter/PVFit" ]       # fit with all constraints I can think of

tuple.B.addTool(TupleToolDecayTreeFitter("MassFit"))
tuple.B.MassFit.constrainToOriginVertex = False
tuple.B.MassFit.daughtersToConstrain = [ "J/psi(1S)" ]

tuple.B.addTool(TupleToolDecayTreeFitter("PVFit"))
tuple.B.PVFit.Verbose = True
tuple.B.PVFit.constrainToOriginVertex = True
tuple.B.PVFit.daughtersToConstrain = [ "J/psi(1S)" ]

@endcode
    * 
    * - This will produce the following columns for the B (from this tool):
    *     - B_Fit_status B_Fit_nDOF B_Fit_chi2_B B_Fit_nIter B_Fit_M B_Fit_MERR B_Fit_P B_Fit_PERR 
    *     - B_MassFit_status B_MassFit_nDOF B_MassFit_chi2_B B_MassFit_nIter B_MassFit_M B_MassFit_MERR B_MassFit_P 
    *       B_MassFit_PERR B_PVFit_status B_PVFit_nDOF B_PVFit_chi2_B B_PVFit_nIter B_PVFit_M B_PVFit_MERR B_PVFit_P 
    *     - B_PVFit_PERR B_PVFit_ctau B_PVFit_ctauErr B_PVFit_decayLength B_PVFit_decayLengthErr B_PVFit_J_psi_1S_ctau  
    *       B_PVFit_J_psi_1S_ctauErr B_PVFit_J_psi_1S_decayLength B_PVFit_J_psi_1S_decayLengthErr B_PVFit_KS0_ctau  
    *       B_PVFit_KS0_ctauErr B_PVFit_KS0_decayLength B_PVFit_KS0_decayLengthErr
    *
    * \sa DecayTreeTuple
    *
    *  @author Yasmine Amhis, Matt Needham (original authors), Patrick Koppenburg (re-implementation)
    *  @date   2010-10-30, 2011-04-01
    *
**/
// $Id: $
#ifndef TUPLETOOLDECAYTREEFITTER_H 
#define TUPLETOOLDECAYTREEFITTER_H 1

// Include files
// from Gaudi
#include "GaudiAlg/GaudiTool.h"
#include "TupleToolBase.h"
#include "Kernel/IParticleTupleTool.h"

// STL
#include <vector>
#include <string>

class IParticleDescendants;
// pid
namespace LHCb{
  class ParticleID;
  class IParticlePropertySvc;
  class VertexBase;
  class Particle;
  class RecVertex ;
}
namespace DecayTreeFitter{
  class Fitter;
}
typedef std::map< std::string, std::vector<double> > TupleMap ; ///< temporary storage of tuple data

class DVAlgorithm;
/** @class TupleToolDecayTreeFitter TupleToolDecayTreeFitter.h
 *  
 * Yasmine Amhis and Matt Needham 
 * 30-10-10
 */
class TupleToolDecayTreeFitter : public TupleToolBase, virtual public IParticleTupleTool {
public: 
  /// Standard constructor
  TupleToolDecayTreeFitter( const std::string& type, 
                            const std::string& name,
                            const IInterface* parent);

  virtual ~TupleToolDecayTreeFitter( ){} ; ///< Destructor

 
  
  StatusCode initialize();


  virtual StatusCode fill( const LHCb::Particle*
                           , const LHCb::Particle*
                           , const std::string&
                           , Tuples::Tuple& );

  
  
 private:
  ///  Fill inforation for a given origin vertex
  StatusCode fit(DecayTreeFitter::Fitter& fitter, 
                 const LHCb::Particle* P,
                 const LHCb::VertexBase* pv,
                 const std::string& prefix, 
                 TupleMap& tMap) const;
  ///  Fill lifetime information
  ///  Fill fit inforation for top decay
  StatusCode fillDecay(const DecayTreeFitter::Fitter& fitter, 
                       const std::string& prefix, 
                       TupleMap& tMap) const;
  ///  Fill lifetime information
  StatusCode fillLT(const DecayTreeFitter::Fitter& fitter, 
                    const LHCb::Particle* P,
                    const std::string& prefix, 
                    TupleMap& tMap) const;

  ///  Fill momentum and mass information
  StatusCode fillMomentum(const DecayTreeFitter::Fitter& fitter, 
                          const LHCb::Particle* P,
                          const std::string& prefix, 
                          TupleMap& tMap) const;

  double sumPT(const LHCb::RecVertex*) const ; ///< sum of pt in pv
  ///  Fill PV information
  StatusCode fillPV(const LHCb::VertexBase* pv,
                    const std::string& prefix, 
                    TupleMap& tMap) const;
  
  ///  Fill lifetime information for non stable daughters
  StatusCode fillDaughters( const DecayTreeFitter::Fitter& fitter,
                            const LHCb::Particle* P,
                            const std::string& prefix,
                            TupleMap& tMap )const;
  /// actual filling of tuple
  StatusCode fillTuple(const TupleMap& tMap, 
                       Tuples::Tuple& tuple, 
                       std::string prefix)const ;
  /// insert helper method
  StatusCode insert(std::string leaf, 
                    double val, 
                    TupleMap& tMap)const ;
  
  std::string getName(int id) const;  ///< name of particle
  
  ///  origin vertex
  std::vector<const LHCb::VertexBase*> originVertex( const  LHCb::Particle*,
                                                     const LHCb::Particle* ) const; 
  
  std::string m_pvLocation ; ///<  PV location to be used. If empty, take context-dependent default

  DVAlgorithm* m_dva;
 
  std::vector<std::string> m_massConstraints; 
  std::vector<LHCb::ParticleID> m_massConstraintsPids;

  bool m_constrainToOriginVertex;   ///< Constrain to Origin Vertex for lifetime

  LHCb::IParticlePropertySvc* m_ppSvc ;
  IParticleDescendants* m_particleDescendants;

  unsigned int m_maxPV ;
  

 };
#endif // TUPLETOOLDECAYTREEFITTER_H
