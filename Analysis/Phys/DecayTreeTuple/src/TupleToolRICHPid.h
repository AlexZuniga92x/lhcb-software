// $Id: TupleToolPid.h,v 1.2 2008/11/11 07:47:58 pkoppenb Exp $
#ifndef APOWELL_TUPLETOOLPID_H
#define APOWELL_TUPLETOOLPID_H 1

// Include files
// from Gaudi
#include "TupleToolBase.h"
#include "Kernel/IParticleTupleTool.h"            // Interface

/** @class TupleToolRICHPid TupleToolRICHPid.h powell/TupleToolRICHPid.h
 *
 * \brief DLL and PID information to be stored in a Tuple
 *
 * \sa DecayTreeTuple
 *
 * For the long lived particles (isBasicParticle()).
 * - head_RICHDLLe : LHCb::ProtoParticle::RICHDLLe
 * - head_RICHDLLmu : LHCb::ProtoParticle::RICHDLLmu
 * - head_RICHDLLK : LHCb::ProtoParticle::RICHDLLk
 * - head_RICHDLLp : LHCb::ProtoParticle::RICHDLLp
 * - head_RICHDLLpi : LHCb::ProtoParticle::RICHDLLpi
 *
 *  @author Andrew Powell
 *  @date   02-Nov-2010
 */
class TupleToolRICHPid : public TupleToolBase, virtual public IParticleTupleTool {
public:
  /// Standard constructor
  TupleToolRICHPid( const std::string& type,
		    const std::string& name,
		    const IInterface* parent);
  
  virtual ~TupleToolRICHPid(){}; ///< Destructor

  virtual StatusCode fill( const LHCb::Particle*, 
			   const LHCb::Particle*, 
			   const std::string&, 
			   Tuples::Tuple& );

private: 

  int m_photonID; 
  int m_pi0ID; 
  
};

#endif // APOWELL_TUPLETOOLRICHPID_H
