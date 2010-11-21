// $Id: HltMoveVerticesForSwimming.cpp,v 1.11 2010-08-17 11:26:35 graven Exp $
// Include files 

// from Gaudi
#include "GaudiKernel/DeclareFactoryEntries.h" 
#include "boost/foreach.hpp"

// from Event
#include "HltMoveVerticesForSwimming.h"

using namespace LHCb;

//-----------------------------------------------------------------------------
// Implementation file for class : HltMoveVerticesForSwimming 
//
// 2008-02-12 : V. Gligorov
//-----------------------------------------------------------------------------

// Declaration of the Algorithm Factory
DECLARE_ALGORITHM_FACTORY( HltMoveVerticesForSwimming );


//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
HltMoveVerticesForSwimming::HltMoveVerticesForSwimming( const std::string& name, ISvcLocator* pSvcLocator)
  : HltAlgorithm ( name , pSvcLocator )
  , m_selections(*this)
{
  declareProperty("Blifetime", 		    m_bLifetime		    = -999999.0	);
  declareProperty("Bip",                m_bIP               = -999999.0 );
  declareProperty("SwimmingDistance", 	m_swimmingDistance	= 1.0		);
  declareProperty("Bcontainer",         m_Bcontainer        = "/Event/SeqD2KK/Phys/SelD2KK");
  declareProperty("OfflinePVs",         m_offlinePVs        = "/Event/Rec/Vertex/Primary");
  
  declareProperty("lifetimeFitter", m_lifeToolName = "PropertimeFitter" );
  declareProperty("distanceCalculator", m_distToolName = "LoKi::DistanceCalculator") ;
  declareProperty("relatedPVFinder", m_finderToolName = "GenericParticle2PVRelator__p2PVWithIPChi2_OfflineDistanceCalculatorName_/P2PVWithIPChi2");
  m_selections.declareProperties();
}

//=============================================================================
// Destructor
//=============================================================================
HltMoveVerticesForSwimming::~HltMoveVerticesForSwimming() {} 

//=============================================================================
// Initialization
//=============================================================================
StatusCode HltMoveVerticesForSwimming::initialize() {

  StatusCode sc = HltAlgorithm::initialize(); // must be executed first
  if ( sc.isFailure() ) return sc;  // error printed already by GaudiAlgorithm

  m_selections.retrieveSelections();
  m_selections.registerSelection();

  m_fit = tool<ILifetimeFitter>( m_lifeToolName, this );
  if( !m_fit ){
    Error("Unable to retrieve the ILifetimeFitter tool");
    return StatusCode::FAILURE;
  }

  m_dist = tool<IDistanceCalculator>( m_distToolName, this );
  if( !m_dist ){
    Error("Unable to retrieve the IDistanceCalculator tool");
    return StatusCode::FAILURE;
  }

  m_finder = tool<IRelatedPVFinder>( m_finderToolName, this );
  if( !m_finder ){
    Error("Unable to retrieve the IRelatedPVFinder tool");
    return StatusCode::FAILURE;
  }

  return sc;
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode HltMoveVerticesForSwimming::execute() {

  StatusCode sc = StatusCode::SUCCESS;

  m_selections.output()->clean(); //TODO: is this really needed?

  debug() << "About to get the offline particles" << endmsg;

  //Check if particles exist
  if (!exist<Particles>(m_Bcontainer+"/Particles")) return sc;

  debug() << "The particles exist, will now get them" << endmsg;

  Particles* pars = get<Particles>(m_Bcontainer+"/Particles");
  if (msgLevel(MSG::DEBUG)) {
    if (pars == 0) verbose() << " no particles found! " << endmsg;
    else verbose() << " particles found " << pars->size() << endmsg;
  }   
  if (pars == 0) return sc;
  
  debug() << "About to check if only one particle in the event" << endmsg;
  //If more than one particle quit!
  if (pars->size() != 1) return sc;
  debug() << "About to check if there are any PVs in the event!" << endmsg;
  //If no PVs quit!
  if (m_selections.input<1>()->empty()) return sc;
  
  //Print the online PVs for debug
  if (msgLevel(MSG::DEBUG)) {
        debug() << "Printing out the input vertices" << endmsg;
        BOOST_FOREACH(const LHCb::RecVertex* v, *m_selections.input<1>()) {
                debug() << *v << endmsg;
        }   
  }

  /*
  debug() << "Check if there are any offline PVs associated to the signal particle" << endmsg; 
  //If no associated offline PVs quit!
  if (!exist<LHCb::Relation1D<LHCb::Particle,LHCb::VertexBase> >(m_Bcontainer+"/Particle2VertexRelations")) return sc;
  //Get the relations
  debug() << "Get the relations table!" << endmsg;
  LHCb::Relation1D<LHCb::Particle,LHCb::VertexBase>* rels = 
    get<LHCb::Relation1D<LHCb::Particle,LHCb::VertexBase> >(m_Bcontainer+"/Particle2VertexRelations");   
  //Get the PV
  debug() << "Get the offline PV associated to the signal" << endmsg;
  LHCb::VertexBase* offPV = rels->relations().begin()->to();
  */

  //Now get the offline PVs
  debug() << "About to get the offline PVs" << endmsg;
  if (!exist<LHCb::RecVertex::Range>(m_offlinePVs)) return sc;
  debug() << "Offline PVs found, grabbing them" << endmsg; 
  const LHCb::RecVertex::Range offPVs = get<LHCb::RecVertex::Range>(m_offlinePVs);

  //Print the offline PVs for debug
  if (msgLevel(MSG::DEBUG)) {
        debug() << "Printing out the offline vertices" << endmsg;
        BOOST_FOREACH(const LHCb::RecVertex* v, offPVs) {
                debug() << *v << endmsg;
        }   
  }

  //Now get the best PV for the particle
  debug() << "About to get the related PV" << endmsg;
  const LHCb::VertexBase* offPV  = m_finder->relatedPV(*(pars->begin()), LHCb::RecVertex::ConstVector(offPVs.begin(), offPVs.end()));
  LHCb::VertexBase* offPV_Clone = offPV->clone();

  sc = move_PVs(*(pars->begin()),offPV_Clone);
  
  int ncan = m_selections.output()->size();
  debug() << " candidates found " << ncan << endmsg;
  return sc;
  
}
//=============================================================================
StatusCode HltMoveVerticesForSwimming::move_PVs(LHCb::Particle* myB, LHCb::VertexBase* offPV){
//Move the PVs

  StatusCode sc = StatusCode::SUCCESS;  

  /*
  double vtxchi2,minvtxchi2 = 999999999.;
  const LHCb::RecVertex* bestVertex;
  //Get the online PV matching best to the offline PV 
  BOOST_FOREACH(const LHCb::RecVertex* vertex, *m_selections.input<1>() ) {
    vtxchi2 = sqrt  (
                        pow(vertex->position().x() - offPV->position().x(),2) +
                        pow(vertex->position().y() - offPV->position().y(),2) +
                        pow(vertex->position().z() - offPV->position().z(),2) 
                    );
    if (vtxchi2<minvtxchi2) {
        minvtxchi2 = vtxchi2;
        bestVertex = vertex;
    }
  } 
  */ 

  //I know this shouldn't modify the input vertex in the long run,
  //but for now it is easier to do it like this 
  BOOST_FOREACH(const LHCb::RecVertex* vertex, *m_selections.input<1>() ) {
    debug() << "The primary vertex is at " << vertex << endmsg;
    debug() << "With X coordinate " << vertex->position().X() << endmsg;
    debug() << "With Y coordinate " << vertex->position().Y() << endmsg;
    debug() << "With Z coordinate " << vertex->position().Z() << endmsg;
    
    const_cast<LHCb::RecVertex*>(vertex)->setPosition(vertex->position() + m_swimmingDistance*myB->slopes().Unit());
    m_selections.output()->push_back( vertex ); 
 
    debug() << "The new vertex" << endmsg;
    debug() << "With X coordinate " << vertex->position().X() << endmsg;
    debug() << "With Y coordinate " << vertex->position().Y() << endmsg;
    debug() << "With Z coordinate " << vertex->position().Z() << endmsg;

  }
  //Also move the clone of the offline PV to compute the new lifetime
  offPV->setPosition(offPV->position() + m_swimmingDistance*myB->slopes().Unit());

  double pt,ept,chi2,ip,ipchi2 = -99999999.;
  sc =  m_fit->fit ( *offPV, *myB , pt, ept, chi2 );
  if (!sc) {  
    warning() << "The lifetime fit failed!!" << endmsg; 
    return sc;
  }
  debug() << "The lifetime of the signal candidate is now = " << pt << endmsg;
  m_bLifetime = pt;

  sc =  m_dist->distance ( myB , offPV, ip, ipchi2 );
  if (!sc) {   
    warning() << "The distance calculator failed!!" << endmsg; 
    return sc; 
  }
  debug() << "The IP of the signal candidate is now = " << ip << endmsg;
  m_bIP = ip;

  return sc;
}
