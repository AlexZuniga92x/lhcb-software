// $Id: ParticleStuffer.cpp,v 1.8 2006-12-06 14:57:34 jpalac Exp $
// Include files 

// 
#include <math.h>

// from Gaudi
#include "GaudiKernel/ToolFactory.h"
#include "GaudiKernel/IParticlePropertySvc.h"
#include "GaudiKernel/ParticleProperty.h"

// local
#include "ParticleStuffer.h"

//-----------------------------------------------------------------------------
// Implementation file for class : ParticleStuffer
//
// 15/03/2002 : Paul Colrain
//-----------------------------------------------------------------------------

// Declaration of the Tool Factory
static const  ToolFactory<ParticleStuffer>          s_factory ;
const        IToolFactory& ParticleStufferFactory = s_factory ; 


//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
ParticleStuffer::ParticleStuffer( const std::string& type,
                                  const std::string& name,
                                  const IInterface* parent )
  : GaudiTool ( type, name , parent )
  , m_ppSvc(0)
  , m_pTransporter(0) 
  , m_transporterType("ParticleTransporter:PUBLIC"){

  // declare additional Interface
  declareInterface<IParticleStuffer>(this);
  declareProperty("Transporter", m_transporterType);
}

//=============================================================================

StatusCode ParticleStuffer::initialize() {
  StatusCode sc = GaudiTool::initialize();
  if (!sc) return sc;
  
  // This tool needs to use internally the ParticlePropertySvc to retrieve the
  // mass to be used
  m_ppSvc = svc<IParticlePropertySvc>("ParticlePropertySvc");
  m_pTransporter = tool<IParticleTransporter>(m_transporterType, this);

  return StatusCode::SUCCESS;
  
}
//=============================================================================
// Stuffer
//=============================================================================
StatusCode ParticleStuffer::fillParticle( const LHCb::Particle::ConstVector& daughters,
                                          const LHCb::Vertex& vtx, 
                                          const LHCb::ParticleID& pid,
                                          LHCb::Particle& part){
  part.setParticleID(pid);
  return fillParticle(daughters,vtx,part);
}
//=============================================================================
// Stuffer
//=============================================================================
StatusCode ParticleStuffer::fillParticle( const LHCb::Particle::ConstVector& daughters,
                                          const LHCb::Vertex& vtx, 
                                          LHCb::Particle& part){

  double zVtxPos = vtx.position().z();
  // Set the four-momenta
  Gaudi::XYZTVector MotherlorVec ;
  Gaudi::SymMatrix4x4 MothermeMat ; 

  LHCb::Particle transParticle;
  for (LHCb::Particle::ConstVector::const_iterator it = daughters.begin(); 
       it != daughters.end(); ++it ) {

    // Transport Particle parameters to the vertex position	 
    StatusCode sctrans = m_pTransporter->transport((*it),zVtxPos,transParticle);
    if ( !sctrans.isSuccess() ) {
      debug() << "Particle extrapolation failed" << endmsg;
      return sctrans;
    }
    MotherlorVec += transParticle.momentum(); 
    // Set the four-momentum error matrix.
    MothermeMat += transParticle.momCovMatrix();    
  }
  part.setMomentum( MotherlorVec );
  part.setMomCovMatrix( MothermeMat );

  // Set the measured mass.
  part.setMeasuredMass( part.momentum().mag() ); 

  // Set the error on the measured mass.
  const Gaudi::Vector4 derivs(- part.momentum().X() / part.measuredMass(),
                              - part.momentum().Y() / part.measuredMass(),
                              - part.momentum().Z() / part.measuredMass(),
                                part.momentum().E() / part.measuredMass()  );

  const double massErr2 = ROOT::Math::Similarity<double,4>(derivs, 
                                                           part.momCovMatrix());
  
  const double massErr = sqrt(massErr2);
  
  part.setMeasuredMassErr( massErr ); 

  // Set the point on track.
  part.setReferencePoint( vtx.position() ); 

  // Set the point on track error matrix.
  part.setPosCovMatrix( vtx.covMatrix() ); 

  // Set the point - four-momentum error matrix.
  // part.setPosMomCorr( pmeMat );
  // Set the end vertex reference.
  part.setEndVertex( &vtx ); 

  // Set the point - slopes+momentum error matrix. Already done internally
  // by Particle via setPosMomCorr
//    HepMatrix pseMat( 3, 3, 0 );  
//    part.setPosSlopesCorr( pseMat );
  
  return StatusCode::SUCCESS;
}
