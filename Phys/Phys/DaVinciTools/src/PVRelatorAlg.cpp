// $Id: PVRelatorAlg.cpp,v 1.3 2009-05-23 18:36:51 jpalac Exp $
// Include files 

// from Gaudi
#include "GaudiKernel/AlgFactory.h" 
// from LHCb
#include "Kernel/IOnOffline.h"
#include "Kernel/IRelatedPVFinder.h"
#include "Event/RecVertex.h"
#include "Event/Particle.h"
#include "Kernel/Particle2Vertex.h"

// local
#include "PVRelatorAlg.h"

//-----------------------------------------------------------------------------
// Implementation file for class : PVRelatorAlg
//
// 2009-05-23 : Juan Palacios
//-----------------------------------------------------------------------------

// Declaration of the Algorithm Factory
DECLARE_ALGORITHM_FACTORY( PVRelatorAlg );


//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
PVRelatorAlg::PVRelatorAlg( const std::string& name,
                            ISvcLocator* pSvcLocator)
  : 
  GaudiAlgorithm ( name , pSvcLocator ),
  m_particleInputLocation(""),
  m_PVInputLocation(LHCb::RecVertexLocation::Primary),
  m_P2PVInputLocation(""),
  m_P2PVOutputLocation(""),
  m_useTable(false),
  m_OnOffline(0),
  m_pvRelator(0)
{
  declareProperty("ParticleInputLocation",  m_particleInputLocation);
  declareProperty("PrimaryVertexInputLocation",  m_PVInputLocation);
  declareProperty("P2PVRelationsInputLocation",  m_P2PVInputLocation);
  //  declareProperty("P2PVRelationsOutputLocation",  m_P2PVOutputLocation);
  
}
//=============================================================================
// Destructor
//=============================================================================
PVRelatorAlg::~PVRelatorAlg() {} 

//=============================================================================
// Initialization
//=============================================================================
StatusCode PVRelatorAlg::initialize() {

  StatusCode sc = GaudiAlgorithm::initialize(); // must be executed first
  if ( sc.isFailure() ) return sc;  // error printed already by GaudiAlgorithm

  if ( msgLevel(MSG::DEBUG) ) debug() << "==> Initialize" << endmsg;
  if (m_P2PVInputLocation!="") m_useTable=true;
  
  if ( m_useTable && 
       ( m_particleInputLocation!="" ||
         m_PVInputLocation!=LHCb::RecVertexLocation::Primary) ) {
    return Error("You have set P2PVRelationsLocation and one of ParticleInputLocation and PrimaryVertexInputLocation.",
                 StatusCode::FAILURE, 
                 1 );    
  }
  
  m_OnOffline = tool<IOnOffline>("OnOfflineTool",this);
  if (0==m_OnOffline) return Error("Failed to get IOnOffline tool");
  
  m_pvRelator = tool<IRelatedPVFinder>(m_OnOffline->relatedPVFinderType(), 
                                       this);
 
  if (0==m_pvRelator) return Error("Failed to get IRelatedPVFinder tool");

  return StatusCode::SUCCESS;
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode PVRelatorAlg::execute() {

  if ( msgLevel(MSG::DEBUG) ) debug() << "==> Execute" << endmsg;

  return StatusCode::SUCCESS;
}
//=============================================================================
const Particle2Vertex::Table* PVRelatorAlg::table() 
{
  typedef LHCb::Particle::Container Particles;
  typedef LHCb::RecVertex::Container Vertices;
  typedef Particle2Vertex::LightTable RelTable;  

  const Particles* particles = i_get<Particles>(m_particleInputLocation);

  const Vertices* vertices = i_get<Vertices>(m_PVInputLocation);

  Particle2Vertex::Table* table = new Particle2Vertex::Table();

  if (0==particles || 0==vertices) return table;

  for (Particles::const_iterator iPart = particles->begin();
       iPart != particles->end(); ++ iPart) {
    const RelTable::Range range = 
      m_pvRelator->relatedPVs(*iPart, *vertices).relations();
    table->merge(range);
  }  
  return table;
 
}
//=============================================================================
//=============================================================================
const Particle2Vertex::Table* PVRelatorAlg::tableFromTable() 
{
  typedef LHCb::Particle::ConstVector Particles;
  typedef LHCb::RecVertex::ConstVector Vertices;
  typedef Particle2Vertex::LightTable RelTable;
  typedef Particle2Vertex::Table Table;

  const Table* inputTable = i_get<Table>(m_P2PVInputLocation);

  Table* table = new Table();

  if (0==table) return table;

  Table::Range range = inputTable->relations();

  Particles particles;
  Vertices vertices;
  
//   for (Table::Range::const_iterator iRange = range.begin();
//        iRange != range.end(); ++iRange) {
//     const LHCb::Particle* particle = iRange->from() ;
//   }

//   for (Particles::const_iterator iPart = particles.begin();
//        iPart != particles.end(); ++ iPart) {
//     const Table::Range range = inputTable.relations(*iPart);
//     for (Table::Range::const_iterator iRange = range.begin();
//          iRange != range.end();
//          ++iRange) {
//       vertices.push_back(iRange->to() );
//     }
//   }

  return table;


}
//=============================================================================
//=============================================================================
//  Finalize
//=============================================================================
StatusCode PVRelatorAlg::finalize() {

  if ( msgLevel(MSG::DEBUG) ) debug() << "==> Finalize" << endmsg;

  return GaudiAlgorithm::finalize();  // must be called after all other actions
}
//=============================================================================
