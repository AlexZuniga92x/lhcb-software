// Include files

// from Gaudi
#include "GaudiKernel/ToolFactory.h"

// local
#include "TupleToolVtxIsoln.h"

#include <Kernel/GetIDVAlgorithm.h>
#include <Kernel/IDVAlgorithm.h>
#include <Kernel/IDistanceCalculator.h>
#include <Kernel/IVertexFit.h>
#include "GaudiAlg/Tuple.h"
#include "GaudiAlg/TupleObj.h"

#include "Event/Particle.h"

using namespace LHCb;

//-----------------------------------------------------------------------------
// Implementation file for class : TupleToolVtxIsoln
//
// @author Mitesh Patel, Patrick Koppenburg
// @date   2008-04-15
//-----------------------------------------------------------------------------

// Declaration of the Tool Factory
// actually acts as a using namespace TupleTool
DECLARE_TOOL_FACTORY( TupleToolVtxIsoln )

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
  TupleToolVtxIsoln::TupleToolVtxIsoln( const std::string& type,
                                        const std::string& name,
                                        const IInterface* parent )
    : TupleToolBase ( type, name , parent )
    , m_dva(0)
    , m_dist(0)
    , m_pVertexFit(0)
{
  declareInterface<IParticleTupleTool>(this);

  m_inputParticles.push_back("/Event/Phys/StdNoPIDsPions");

  declareProperty( "MaxDeltaChi2", m_deltaChi2 = 9.0);
  declareProperty( "MaxChi2", m_Chi2 = 9.0);
  declareProperty( "VertexFit", m_typeVertexFit = "default");
  declareProperty("InputParticles", m_inputParticles );
}

//=============================================================================

StatusCode TupleToolVtxIsoln::initialize()
{
  const StatusCode sc = TupleToolBase::initialize();
  if ( sc.isFailure() ) return sc;

  m_dva = Gaudi::Utils::getIDVAlgorithm ( contextSvc(), this ) ;
  if (!m_dva) return Error("Couldn't get parent DVAlgorithm",
                           StatusCode::FAILURE);

  m_dist = m_dva->distanceCalculator ();
  if( !m_dist ){
    Error("Unable to retrieve the IDistanceCalculator tool");
    return StatusCode::FAILURE;
  }

  m_pVertexFit= m_dva->vertexFitter();
  if( !m_pVertexFit ){
    Error("Unable to retrieve the IVertexFit tool");
    return StatusCode::FAILURE;
  }

  return sc;
}

//=============================================================================

StatusCode TupleToolVtxIsoln::fill( const Particle* mother
                                    , const Particle* P
                                    , const std::string& head
                                    , Tuples::Tuple& tuple )
{

  const std::string prefix=fullName(head);
  Assert( P && mother && m_dist
          , "This should not happen, you are inside TupleToolVtxIsoln.cpp :(" );


  bool test=true;

  // --------------------------------------------------

  // find the origin vertex. Either the primary or the origin in the
  // decay
  /*
    const VertexBase* vtx = 0;
    if( mother != P ) vtx = originVertex( mother, P );
    if( !vtx ){
    Error("Can't retrieve the origin vertex for " + prefix );
    return StatusCode::FAILURE;
    }
  */
  const LHCb::Vertex* vtx;
  if (P->isBasicParticle() || isPureNeutralCalo(P) ){
    vtx = mother->endVertex();
  }
  else{
    vtx = P->endVertex();
  }
  debug()<<"vertex for P, ID " <<P->particleID().pid()<<" = " <<vtx<<" at "<<vtx->position()<<  endmsg;
  if( !vtx ){
    Error("Can't retrieve the  vertex for " + prefix );
    return StatusCode::FAILURE;
  }

  // The vertex chi2 of the composite particle being tested
  double vtxChi2 = vtx->chi2();

  //--------------------------------------------------
  // Get all the particle's final states
  LHCb::Particle::ConstVector source;
  LHCb::Particle::ConstVector target;
  LHCb::Particle::ConstVector finalStates;
  LHCb::Particle::ConstVector parts2Vertex;
  parts2Vertex.clear();

  //   const LHCb::Particle* prefix = P;
  if (P->isBasicParticle()){
    source.push_back(mother);
  }
  else{
    source.push_back(P);
  }
  // The first iteration is for the particle to filter, which has an endVertex
  do {
    target.clear();
    for(LHCb::Particle::ConstVector::const_iterator isource = source.begin();
        isource != source.end(); isource++){

      if(!((*isource)->daughters().empty())){

        LHCb::Particle::ConstVector tmp = (*isource)->daughtersVector();

        for( LHCb::Particle::ConstVector::const_iterator itmp = tmp.begin();
             itmp!=tmp.end(); itmp++){
          target.push_back(*itmp);

          // Add the final states, i.e. particles with proto and ignoring gammas
          if((*itmp)->proto() && !isPureNeutralCalo(*itmp) ) finalStates.push_back(*itmp);
        }
      } // if endVertex
    } // isource
    source = target;
  } while(target.size() > 0);

  if (msgLevel(MSG::DEBUG)) debug() << "Final states size= " <<  finalStates.size()  << endreq;


  //--------------------------------------------------
  // Build vector of particles, excluding signal
  LHCb::Particle::ConstVector theParts;


  for(std::vector<std::string>::iterator i = m_inputParticles.begin();
      i !=m_inputParticles.end(); ++i){

    if (!exist<LHCb::Particle::Range>(*i+"/Particles")){
      if (msgLevel(MSG::DEBUG)) debug() << "No particles at " << *i << " !!!!!" << endreq;
      continue;
    }

    LHCb::Particle::Range parts = get<LHCb::Particle::Range>(*i+"/Particles");

    if (msgLevel(MSG::DEBUG)) debug() << "Getting particles from " << *i
                                      << " with " << (parts).size() << " particles" << endreq;

    for(LHCb::Particle::Range::const_iterator iparts = (parts).begin();
        iparts != (parts).end(); ++iparts)
    {

      // Ignore if no proto
      if(!(*iparts)->proto()) continue;
      // Ignore if proto and gammas
      if(isPureNeutralCalo(*iparts)) continue;

      // Compare protos, not pointers (e.g. because of clones)
      bool isSignal = false;
      for(LHCb::Particle::ConstVector::const_iterator signal = finalStates.begin();
          signal != finalStates.end(); ++signal){
        const LHCb::ProtoParticle* orig = (*signal)->proto();
        if(!orig) continue;
        if(orig != (*iparts)->proto()) continue;
        isSignal = true;
        break;
      }

      // Ignore if it is a signal particle
      if(isSignal) continue;

      // Check that the same particle does not appear twice
      bool isIn = false;
      for(LHCb::Particle::ConstVector::const_iterator result = theParts.begin();
          result != theParts.end(); ++result){
        const LHCb::ProtoParticle* orig = (*result)->proto();
        if(!orig) continue;
        if(orig != (*iparts)->proto()) continue;
        isIn = true;
        break;
      }

      // Already in ?
      if(isIn) continue;
      if(!isIn) theParts.push_back(*iparts);

    } // iparts
  } // m_inputParticles

  if (msgLevel(MSG::DEBUG)) debug() << "Number of particles to check excluding signal, particles with same proto and gammas = "
                                    << theParts.size() << endreq;
  //--------------------------------------------------

  /*Now we want to make a list of daughters

  */

  //if the daughter
  if (P->isBasicParticle()){
    parts2Vertex.push_back(P);
  }
  else{
    parts2Vertex = P->daughtersVector();
  }

  if (msgLevel(MSG::DEBUG))
    debug() <<"Now final states should include only your particles direct desendents. finalStates.size()= "
            <<  finalStates.size()  <<endreq;


  int nCompatibleDeltaChi2 = 0;
  int nCompatibleChi2= 0;
  double smallestChi2 = -1 ;
  double smallestDeltaChi2 = -1 ;

  // Now count how many particles are compatible with the vertex of the particle under study
  for(LHCb::Particle::ConstVector::const_iterator iparts = theParts.begin();
      iparts != theParts.end(); ++iparts){


    StatusCode sc = StatusCode::SUCCESS;
    LHCb::Vertex vtxWithExtraTrack;

    // Temporarily add the extra track to the parts2Vertex
    parts2Vertex.push_back(*iparts);
    // if (msgLevel(MSG::DEBUG)) debug() << "Adding trk pid"  << (*iparts)->particleID().pid() << " to vtx" << endmsg;

    // Fit
    sc = m_pVertexFit->fit (vtxWithExtraTrack,parts2Vertex);
    // replaced by V.B. 20.Aug.2k+9: (parts2Vertex,vtxWithExtraTrack);
    // Remove the added track from parts2Vertex

    parts2Vertex.pop_back();
    if (!sc){
      Warning("Failed to fit vertex").ignore();
      // return sc;
    } else{
      double dChi2 = vtxWithExtraTrack.chi2() - vtxChi2 ;
      //fabs(vtxChi2 - vtxWithExtraTrack.chi2())  < m_deltaChi2 is not useful if the particle is basic
      if(dChi2  < m_deltaChi2 &&
         m_deltaChi2 > 0.0 && (! P->isBasicParticle()) ) nCompatibleDeltaChi2++;
      if( (vtxWithExtraTrack.chi2()  < m_Chi2) && ( m_Chi2 > 0.0)  ) nCompatibleChi2++;
      if (msgLevel(MSG::DEBUG)) debug() << "Fitted vertex adding track has Delta chi2 = "
                                        << dChi2  << "chi2 = "
                                        << vtxWithExtraTrack.chi2() << endmsg;
      if ( smallestChi2 < 0 || smallestChi2 > vtxWithExtraTrack.chi2())  smallestChi2 =  vtxWithExtraTrack.chi2();
      if ( smallestDeltaChi2 < 0 || smallestDeltaChi2 > dChi2)  smallestDeltaChi2 = dChi2;
    }

  } // iparts
  if ( P->isBasicParticle()) nCompatibleDeltaChi2=-1;

  if ( msgLevel(MSG::DEBUG) ){
    if(m_deltaChi2 > 0.0)  debug() << "Number of particles with delta chi2 < cut Delta chi2 = "
                                   << nCompatibleDeltaChi2 << endreq;
    if(m_Chi2 > 0.0)  debug() << "Number of particles with  chi2 < cut  chi2 = "
                              << nCompatibleChi2 << endreq;
  }

  if(m_deltaChi2 > 0.0) test &= tuple->column( prefix + "_NOPARTWITHINDCHI2WDW",  nCompatibleDeltaChi2);
  if(m_Chi2 > 0.0)      test &= tuple->column( prefix + "_NOPARTWITHINCHI2WDW",  nCompatibleChi2);
  tuple->column( prefix + "_SMALLESTCHI2",  smallestChi2 );
  tuple->column( prefix + "_SMALLESTDELTACHI2",  smallestDeltaChi2 );

  return StatusCode(test);
}

//=========================================================================
//
//=========================================================================
const Vertex* TupleToolVtxIsoln::originVertex( const Particle* top
                                               , const Particle* P ) const {
  if( top == P || P->isBasicParticle() ) return 0;

  const SmartRefVector< LHCb::Particle >& dau = top->daughters ();
  if( dau.empty() ){
    // if (msgLevel(MSG::DEBUG)) debug() << " Particle has no daughters! "  << endreq;
    return 0;
  }

  SmartRefVector< LHCb::Particle >::const_iterator it;
  for( it = dau.begin(); dau.end()!=it; ++it ){
    if( P == *it ){ // I found the daughter
      return top->endVertex();
    }
  }

  // vertex not yet found, get deeper in the decay:
  for( it = dau.begin(); dau.end()!=it; ++it ){
    if( P != *it && !(*it)->isBasicParticle() ){
      const Vertex* vv = originVertex( *it, P );
      if( vv ) return vv;
    }
  }
  return 0;
}
