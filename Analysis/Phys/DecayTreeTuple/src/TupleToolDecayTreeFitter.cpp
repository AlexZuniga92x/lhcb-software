// $Id: $
// Include files 

// from Gaudi
#include "GaudiKernel/ToolFactory.h" 
// local
#include "TupleToolDecayTreeFitter.h"
#include <Kernel/GetDVAlgorithm.h>
#include <Kernel/DVAlgorithm.h>
#include "GaudiAlg/Tuple.h"
#include "GaudiAlg/TupleObj.h"
#include "Event/RecVertex.h"
#include "Event/Particle.h"
#include "Kernel/ParticleProperty.h"
#include "Kernel/IParticleDescendants.h"
#include "Kernel/Escape.h"
#include "DecayTreeFitter/Fitter.h"
#include "Kernel/IParticlePropertySvc.h"

using namespace LHCb;

//-----------------------------------------------------------------------------
// Implementation file for class : TupleToolDecayTreeFitter
// Yasmine Amhis, Matt Needham, Patrick Koppenburg
// 30-10-2010, 01-04-2011 
//-----------------------------------------------------------------------------

// Declaration of the Tool Factory
DECLARE_TOOL_FACTORY( TupleToolDecayTreeFitter );


//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
TupleToolDecayTreeFitter::TupleToolDecayTreeFitter( const std::string& type,
                                                    const std::string& name,
                                                    const IInterface* parent )
  : TupleToolBase ( type, name , parent )
  , m_dva()
  , m_ppSvc()
  , m_particleDescendants()
  
{
  declareProperty("daughtersToConstrain", m_massConstraints , "List of particles to contrain to mass");
  declareProperty("constrainToOriginVertex", m_constrainToOriginVertex = false, 
                  "Do a refit constraining to Origin Vertex (could be PV)");
  declareProperty("MaxPV", m_maxPV = 10  , "Maximal number of PVs considered");
  declareInterface<IParticleTupleTool>(this);
  
}
//=============================================================================
StatusCode TupleToolDecayTreeFitter::initialize()
{
  if( ! TupleToolBase::initialize() ) return StatusCode::FAILURE;
  
   
  // convert the list of names to a list of pids 
  const LHCb::ParticleProperty* prop = 0 ;
  m_ppSvc = svc<LHCb::IParticlePropertySvc>("LHCb::ParticlePropertySvc",true) ;
  for (std::vector<std::string>::const_iterator iterS = m_massConstraints.begin(); 
       iterS != m_massConstraints.end(); ++iterS ){
    prop = m_ppSvc->find( *iterS );
    if (!prop)  Exception("Unknown PID");
    m_massConstraintsPids.push_back(prop->pdgID());
  } // iterS

  m_dva = Gaudi::Utils::getDVAlgorithm ( contextSvc() ) ;
  if (0==m_dva) return Error("Couldn't get parent DVAlgorithm", StatusCode::FAILURE);

  m_particleDescendants = tool<IParticleDescendants> ( "ParticleDescendants");

  if ("" == m_extraName ){ 
    std::string en = name() ; // use tool name as prepended name
    unsigned int d = en.find_last_of(".");
    m_extraName = en.substr(d+1,en.size()-1); // from d to end
    if ( "TupleToolDecayTreeFitter" == m_extraName )  m_extraName = ""; // user has not chanegd instance name
    info() << "All fields will be prepended with ``" << m_extraName << "''" <<endmsg;
  }
  
  return StatusCode::SUCCESS;
}

//=============================================================================
//  The fill method implementation
//=============================================================================
StatusCode TupleToolDecayTreeFitter::fill( const LHCb::Particle* mother
                                           , const LHCb::Particle* P
                                           , const std::string& head
                                           , Tuples::Tuple& tuple ){

  if( !P ) return StatusCode::FAILURE;
  if( P->isBasicParticle() ) {
    Error("Do not call TupleToolDecayTreeFitter for basic particles. Use Branches. See doxygen.");
    return StatusCode::FAILURE;
  }
  const std::string prefix=fullName(head);

  TupleMap tMap ; // the temporary data map
  DecayTreeFitter::Fitter* fitter = 0 ;

  // get origin vertices
  std::vector<const VertexBase*> originVtx;
  if (m_constrainToOriginVertex){  
    if (msgLevel(MSG::DEBUG)) debug() << "Constrain the origin vertex" << endmsg; 
    // check for origin vertex
    originVtx = originVertex( mother, P ); 
    if( originVtx.empty() ){
      Error("Can't get an origin vertex");
      return StatusCode::FAILURE;
    }
    for (std::vector<const VertexBase*>::const_iterator iv = originVtx.begin() ; iv != originVtx.end() ; iv++){
      fitter = new DecayTreeFitter::Fitter(*P, *(*iv));
      if (!fit(fitter,P,*iv,prefix,tMap)) return StatusCode::FAILURE ;
      delete fitter ;
    }
  } else {
    if (msgLevel(MSG::DEBUG)) debug() << "Do not contrain the origin vertex" << endmsg; 
    // Get the fitter
    fitter = new DecayTreeFitter::Fitter(*P);
    if (!fit(fitter,P,0,prefix,tMap)) return StatusCode::FAILURE;
    delete fitter ;
  }
  
  return fillTuple(tMap,tuple,prefix); // the actual filling
}
//=============================================================================
// do filling for a give vertex
//=============================================================================
StatusCode TupleToolDecayTreeFitter::fit(DecayTreeFitter::Fitter* fitter, 
                                         const LHCb::Particle* P,
                                         const LHCb::VertexBase* pv,
                                         const std::string& prefix, 
                                         TupleMap& tMap) const{  
  bool test = true ;
  //add mass contraints
  if (!m_massConstraintsPids.empty()){
    for (std::vector<LHCb::ParticleID>::const_iterator iterC = m_massConstraintsPids.begin();  
         iterC != m_massConstraintsPids.end(); ++iterC) fitter->setMassConstraint(*iterC);
  }
  // fit
  fitter->fit();
  // fill the fit result
  fillDecay(fitter,P,prefix,tMap );
  if (m_constrainToOriginVertex){
    fillPV(pv,prefix,tMap);
    fillLT(fitter,P,prefix,tMap );
    test &= fillDaughters(fitter,P,prefix,tMap ); 
  }
  return StatusCode(test);
}
//=============================================================================
// Fill standard stuff
//=============================================================================
StatusCode TupleToolDecayTreeFitter::fillPV(const LHCb::VertexBase* pv,
                                          const std::string& prefix, 
                                          TupleMap& tMap ) const{
  bool test = true;
  if (!pv) Exception("Null PVs cannot happen!");
  test &= insert( prefix+"_PV_key", pv->key(), tMap );
  if (isVerbose()){
    test &= insert( prefix+"_PV_X", pv->position().X(), tMap );
    test &= insert( prefix+"_PV_Y", pv->position().Y(), tMap );
    test &= insert( prefix+"_PV_Z", pv->position().Z(), tMap );
    if (pv->isPrimary()) test &= insert( prefix+"_PV_sumPT", sumPT(dynamic_cast<const LHCb::RecVertex*>(pv)), tMap );
  }
  return StatusCode(test);
}
//=============================================================================
// Fill standard stuff
//=============================================================================
StatusCode TupleToolDecayTreeFitter::fillDecay(const DecayTreeFitter::Fitter* fitter, 
                                          const Particle* P,
                                          const std::string& prefix, 
                                          TupleMap& tMap ) const{
   
  bool test = true; 

  test &= insert( prefix+"_status", fitter->status(), tMap );
  test &= insert( prefix+"_nDOF", fitter->nDof(), tMap  );
  test &= insert( prefix+"_chi2_B", fitter->chiSquare(), tMap  );
  test &= insert( prefix+"_nIter", fitter->nIter(), tMap  );
     
  //Get the fit parameters
  const Gaudi::Math::ParticleParams* params = fitter->fitParams(P) ;
  Gaudi::Math::LorentzVectorWithError momentum = params->momentum() ; 
      
  test &= insert( prefix+"_M",  momentum.m().value(), tMap  );
  test &= insert( prefix+"_MERR", momentum.m().error(), tMap );
  test &= insert( prefix+"_P", momentum.p().value(), tMap );
  test &= insert( prefix+"_PERR", momentum.p().error(), tMap ) ;//MeV

  return StatusCode(test);
}
//=============================================================================
// Fill lifetime stuff
//=============================================================================
StatusCode TupleToolDecayTreeFitter::fillLT(const DecayTreeFitter::Fitter* fitter, 
                                          const Particle* P,
                                          const std::string& prefix, 
                                          TupleMap& tMap ) const{
   
  bool test = true; 

  const Gaudi::Math::ParticleParams* tParams = fitter->fitParams(P); 
  Gaudi::Math::ValueWithError decayLength = tParams->decayLength();
  Gaudi::Math::ValueWithError ctau = tParams->ctau();
  test &= insert( prefix+"_ctau", ctau.value(), tMap  );
  test &= insert( prefix+"_ctauErr", ctau.error(), tMap  );
  test &= insert( prefix+"_decayLength", decayLength.value(), tMap  );
  test &= insert( prefix+"_decayLengthErr", decayLength.error(), tMap  );
  
  return StatusCode(test);
}
//=============================================================================
// Fill lifetime information for non stable daughters
//=============================================================================
StatusCode TupleToolDecayTreeFitter::fillDaughters( const DecayTreeFitter::Fitter* fitter
                                                    ,const LHCb::Particle* P
                                                    ,const std::string& prefix
                                                    ,TupleMap& tMap )const{
  bool test = true;
  
  LHCb::Particle::ConstVector daughters = m_particleDescendants->descendants(P);
  if (msgLevel(MSG::DEBUG)) debug() << "for id " << P->particleID().pid() << " daughter size is " << daughters.size() << endmsg;
  if ( daughters.size()==0 ) return test;
  for (LHCb::Particle::ConstVector::iterator it = daughters.begin();it<daughters.end(); it++) {
    const LHCb::Particle* particle = *it;
    if ( particle->isBasicParticle()) continue ;
    unsigned int pid = abs(particle->particleID().pid());
    std::string name = prefix+"_"+getName(pid) ;
    test &= fillLT(fitter,particle,name,tMap);
  }
  return StatusCode(test);
}


//=============================================================================
// append data to TupleMap
//=============================================================================
StatusCode TupleToolDecayTreeFitter::insert(std::string leaf, double val, TupleMap& tMap)const {
  TupleMap::iterator l = tMap.find(leaf);
  if ( l==tMap.end()) {  /// first time this is seen. Create
    std::vector<double> vals;
    vals.push_back(val);
    std::pair<std::string,std::vector<double> > p(leaf,vals);
    tMap.insert(p);
  } else {
    l->second.push_back(val); /// append a to vector
  }
  return StatusCode::SUCCESS ;
}

//=============================================================================
// actual filling of the Tuple
//=============================================================================
StatusCode TupleToolDecayTreeFitter::fillTuple(const TupleMap& tMap, Tuples::Tuple& tuple, std::string prefix)const {
  bool test = true ;
  for (TupleMap::const_iterator t = tMap.begin() ; t!=tMap.end() ; ++t){
    std::string leaf = t->first;
    std::vector<double> data = t->second;
    if (msgLevel(MSG::DEBUG)) 
      debug() << "Filling leaf ``" << leaf << "'' with vector of size " << data.size() << endmsg ;
    if (m_maxPV<data.size()) Exception("Seeing data with too many PVs. This should never happen. E-mail P. Koppenburg.");
    test &= tuple->farray( leaf, data, prefix+"_nPV", m_maxPV);
  }
  return StatusCode(test);
}
//=============================================================================
// get origin vertex
//=============================================================================
std::vector<const VertexBase*> TupleToolDecayTreeFitter::originVertex( const Particle* mother, const Particle* P ) const {
  std::vector<const VertexBase*> oriVx;
  if (mother == P){// the origin vertex is the primary.
    const VertexBase* bpv = m_dva->bestPV( P );
    oriVx.push_back(bpv);
    // all the other ones 
    /// @todo : keep only the related ones
    for (LHCb::RecVertex::Range::const_iterator pv = m_dva->primaryVertices().begin() ; 
         pv!=m_dva->primaryVertices().end() ; ++pv){
      if ( *pv != bpv ) oriVx.push_back(*pv);
      if (oriVx.size()>=m_maxPV){
        Warning("Truncated number of PVs",10,StatusCode::FAILURE).ignore();
        break ;
      }
    }
    Warning("Filled all PVs, including unrelated ones. Fix once https://savannah.cern.ch/task/?19817 is done",
            1,StatusCode::SUCCESS).ignore();
  } else {
    const SmartRefVector< LHCb::Particle >& dau = mother->daughters ();
    if( dau.empty() ) return oriVx ;
    
    for( SmartRefVector< LHCb::Particle >::const_iterator it = dau.begin(); dau.end()!=it; ++it ){
      if( P == *it ){
        oriVx.push_back(mother->endVertex());
        return oriVx ;
      }
    }
  
    // vertex not yet found, get deeper in the decay:
    for( SmartRefVector< LHCb::Particle >::const_iterator it = dau.begin(); dau.end()!=it; ++it ){
      if( P != *it && !(*it)->isBasicParticle() ){
        oriVx = originVertex( *it, P );
        if( !oriVx.empty() ) {
          return oriVx ;  // found
        }
      }
    }
  }
  return oriVx;
}


//=============================================================================
// Convert pid number in names
//=============================================================================
std::string TupleToolDecayTreeFitter::getName(int id) const {
  const LHCb::ParticleProperty* prop = m_ppSvc->find( LHCb::ParticleID(id) );
  if (!prop) Exception("Unknown PID");
  return Decays::escape(prop->name());
}
//=============================================================================
// Sum PT
/// @ todo this should be moved to TupleToolPrimaries
//=============================================================================
double TupleToolDecayTreeFitter::sumPT(const LHCb::RecVertex* pv) const {
  if (!pv) Exception("Not a RecVertex?");
  double spt = 0 ;
  for (SmartRefVector< LHCb::Track >::const_iterator t = pv->tracks().begin() ; 
       t!= pv->tracks().end() ; ++t) {
    spt += (*t)->pt();
  }
  return spt ;
}

