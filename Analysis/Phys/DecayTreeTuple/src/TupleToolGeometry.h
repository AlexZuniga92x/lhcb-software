// $Id: TupleToolGeometry.h,v 1.2 2008-07-11 09:21:04 pkoppenb Exp $
#ifndef JBOREL_TUPLETOOLGEOMETRY_H
#define JBOREL_TUPLETOOLGEOMETRY_H 1

// Include files
// from Gaudi
#include "GaudiAlg/GaudiTool.h"
#include "Kernel/IParticleTupleTool.h"            // Interface

class IContextTool;
class IDistanceCalculator;

namespace LHCb {
  class Particle;
  class Vertex;
};


/** @class TupleToolGeometry TupleToolGeometry.h jborel/TupleToolGeometry.h
 *
 * \brief Fill geometry related informatio for DecayTreeTuple
 *
 * - head_IP : impact parameter with respect to the PhysDesktop::relatedVertex()
 * - head_IPERR : impact parameter error estimate
 * - head_FD : flight distance of composite particle wrt. the origin
 *   associated origin vertex (maybe not the relatedVertex()).
 * - head_FDERR : flight distance error estimate
 * - head_FDS : flight distance significance
 * - head_ENDVERTEX_[X|Y|Z] : decay vertex position for composite particles
 * - head_ENDVERTEX_[X|Y|Z]ERR : decay vertex position error estimate for composite particles
 * - head_ENDVERTEX_CHI2 : decay vertex chi2
 * - head_ENDVERTEX_NDOF : decay vertex nDoF
 *
 *
 * \sa DecayTreeTuple
 *
 *  @author Jeremie Borel
 *  @date   2007-11-07
 */
class TupleToolGeometry : public GaudiTool, virtual public IParticleTupleTool {
public:
  /// Standard constructor
  TupleToolGeometry( const std::string& type,
		     const std::string& name,
		     const IInterface* parent);
  
  virtual ~TupleToolGeometry( ){}; ///< Destructor
  
  virtual StatusCode initialize();
  
  StatusCode fill( const LHCb::Particle*
		   , const LHCb::Particle*
		   , const std::string&
		   , Tuples::Tuple& );

private:

  const LHCb::Vertex* originVertex( const  LHCb::Particle*
				    , const LHCb::Particle* ) const;
  

  IContextTool* m_context;
  const IDistanceCalculator* m_dist;
};
#endif // JBOREL_TUPLETOOLGEOMETRY_H
