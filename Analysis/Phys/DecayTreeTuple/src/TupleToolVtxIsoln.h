#ifndef MPATEL_TUPLETOOLVTXISOLN_H
#define MPATEL_TUPLETOOLVTXISOLN_H 1

// Include files
// from Gaudi
#include "TupleToolBase.h"
#include "Kernel/IParticleTupleTool.h"            // Interface

class DVAlgorithm;
class IDistanceCalculator;
class IVertexFit;

namespace LHCb {
  class Particle;
  class Vertex;
};


/** @class TupleToolVtxIsoln TupleToolVtxIsoln.h 
 *
 * \brief Fill isolation information for DecayTreeTuple
 *
 * - head_NOPARTWITHINDCHI2WDW : no. of non-signal particles that when added to vertex give delta chi2 < specified window 
 * - head_NOPARTWITHINCHI2WDW : no. of non-signal particles that when added to vertex give chi2 < specified window 
 * head_SMALLESTCHI2: chi2 of smallest chi2 combination with any of the input Particles
 * head_SMALLESTDELTACHI2: delta chi2 of smallest delta chi2 combination with any of the input Particles
 * 
 * \sa DecayTreeTuple
 *
 *  @todo Maybe one should get Tracks instead of Particles?
 *
 *  @author Mitesh Patel, Patrick Koppenburg
 *  @date   2008-04-15
 */
class TupleToolVtxIsoln : public TupleToolBase, virtual public IParticleTupleTool {
public:
  /// Standard constructor
  TupleToolVtxIsoln( const std::string& type,
		     const std::string& name,
		     const IInterface* parent);
  
  virtual ~TupleToolVtxIsoln( ){}; ///< Destructor
  
  virtual StatusCode initialize();
  
  StatusCode fill( const LHCb::Particle*
		   , const LHCb::Particle*
		   , const std::string&
		   , Tuples::Tuple& );

private:

  const LHCb::Vertex* originVertex( const  LHCb::Particle*
				    , const LHCb::Particle* ) const;
  

  DVAlgorithm* m_dva;
  const IDistanceCalculator* m_dist;
  const IVertexFit* m_pVertexFit;
  double m_deltaChi2; 
  double m_Chi2; 
  std::string m_typeVertexFit; 
  std::vector<std::string> m_inputParticles;
};
#endif // MPATEL_TUPLETOOLGEOMETRY_H
