// $Id: BestPVAlg.cpp,v 1.2 2010-08-04 14:52:55 jpalac Exp $
// Include files 

// from Gaudi
#include "GaudiKernel/AlgFactory.h" 
// from LHCb
#include "Kernel/IOnOffline.h"
#include "Kernel/IRelatedPVFinder.h"
#include "Event/RecVertex.h"
#include "Event/Particle.h"
#include "Kernel/Particle2Vertex.h"
#include "Relations/Get.h"
// DaVinci
#include "Kernel/DaVinciStringUtils.h"
// local
#include "BestPVAlg.h"

//-----------------------------------------------------------------------------
// Implementation file for class : BestPVAlg
//
// 2010-07-26 : Juan Palacios
//-----------------------------------------------------------------------------

// Declaration of the Algorithm Factory
DECLARE_ALGORITHM_FACTORY( BestPVAlg );


//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
BestPVAlg::BestPVAlg( const std::string& name,
                      ISvcLocator* pSvcLocator)
  : 
  GaudiAlgorithm ( name , pSvcLocator ),
  m_PVInputLocation(LHCb::RecVertexLocation::Primary),
  m_particleInputLocations(),
  m_P2PVInputLocations(),
  m_useTables(false),
  m_OnOffline(0),
  m_pvRelator(0)
{
  declareProperty("PrimaryVertexInputLocation",  m_PVInputLocation);
  declareProperty("ParticleInputLocations",  m_particleInputLocations);
  declareProperty("P2PVRelationsInputLocations",  m_P2PVInputLocations);
  
}
//=============================================================================
// Destructor
//=============================================================================
BestPVAlg::~BestPVAlg() {} 

//=============================================================================
// Initialization
//=============================================================================
StatusCode BestPVAlg::initialize() {

  StatusCode sc = GaudiAlgorithm::initialize(); // must be executed first
  if ( sc.isFailure() ) return sc;  // error printed already by GaudiAlgorithm

  if ( msgLevel(MSG::DEBUG) ) debug() << "==> Initialize" << endmsg;
  if (!m_P2PVInputLocations.empty()) m_useTables=true;
  
  if ( m_useTables ) { 
    if ( (!m_particleInputLocations.empty()) ||
         (m_PVInputLocation!=LHCb::RecVertexLocation::Primary) )  {
      return Error("You have set P2PVRelationsLocations and one of ParticleInputLocations and PrimaryVertexInputLocation.",
                   StatusCode::FAILURE);
    }
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
StatusCode BestPVAlg::execute() {

  if ( msgLevel(MSG::DEBUG) ) debug() << "==> Execute" << endmsg;

  if (m_useTables) {
    this->tablesFromTables();
  } else {
    if (exist< LHCb::RecVertex::Range>(m_PVInputLocation) && 
        !get<LHCb::RecVertex::Range>(m_PVInputLocation).empty()) {
      this->tables();
    }    
  }

  return StatusCode::SUCCESS;
}
//=============================================================================
void BestPVAlg::tables() const
{

  typedef LHCb::Particle::Range Particles;
  typedef LHCb::RecVertex::Range Vertices;

  std::vector<std::string>::const_iterator iLoc = m_particleInputLocations.begin();
  std::vector<std::string>::const_iterator locEnd = m_particleInputLocations.end();

  for ( ; iLoc != locEnd; ++iLoc) {

    if ( exist< Particles >(*iLoc) &&  
         ! get< Particles >(*iLoc).empty()) {

      Particle2Vertex::Table* table = new Particle2Vertex::Table();
      const std::string tableLoc = tableLocation(*iLoc);
      put( table, tableLoc );

      Particles particles = get<Particles>(*iLoc);
      Vertices vertices = get<Vertices>(m_PVInputLocation);

      for (Particles::const_iterator iPart = particles.begin();
           iPart != particles.end(); ++ iPart) {
        const LHCb::VertexBase* vtx = 
          m_pvRelator->relatedPV(*iPart, 
                                 LHCb::VertexBase::ConstVector(vertices.begin(), 
                                                               vertices.end()));
        table->relate(*iPart, vtx);

      }
      checkTable(table, tableLoc);
    }
    
  }

}

//=============================================================================
void BestPVAlg::tablesFromTables() const
{
  typedef LHCb::Particle::ConstVector Particles;
  typedef LHCb::VertexBase::ConstVector Vertices;
  typedef LHCb::RecVertex::ConstVector PVs;
  typedef Particle2Vertex::Table Table;
  typedef Particle2Vertex::Table::InvType InvTable;
 
  std::vector<std::string>::const_iterator iLoc = m_P2PVInputLocations.begin();
  std::vector<std::string>::const_iterator locEnd = m_P2PVInputLocations.end();
 
  for ( ; iLoc != locEnd ; ++iLoc) {

    const Table* inputTable = i_get<Table>(*iLoc);

    if (inputTable) {

      Table* table = new Table();
      const std::string tableLoc = tableLocation(*iLoc);
      put( table,  tableLoc );

      const InvTable invTable(*inputTable, 1);

      InvTable::Range invRange = invTable.relations();

      Particles particles;

      Relations::getUniqueTo(invRange.begin(), invRange.end(), particles);

      if ( msgLevel(MSG::VERBOSE) ) {
        verbose() << "tableFromTable found " 
                  << particles.size() << " Particles" 
                  << endmsg;
      }
  
      for (Particles::const_iterator iPart = particles.begin();
           iPart != particles.end(); ++iPart) {

        const Table::Range range = inputTable->relations(*iPart);

        Vertices vertices;

        Relations::getUniqueTo(range.begin(), range.end(), vertices);

        if ( msgLevel(MSG::VERBOSE) ) verbose() << "tableFromTable found " 
                                                << vertices.size() 
                                                << " related vertices" << endmsg;

        const LHCb::VertexBase* vtx = m_pvRelator->relatedPV(*iPart, 
                                                             LHCb::VertexBase::ConstVector(vertices.begin(), 
                                                                                           vertices.end()));

        table->relate(*iPart, vtx);
      }
      checkTable(table, tableLoc);
    } // if inputTable 

  } // location loop
  

}
//=============================================================================
std::string BestPVAlg::tableLocation(const std::string& location) const
{
  std::string tableLocation(location);
  DaVinci::StringUtils::removeEnding(tableLocation, "/");
  return tableLocation + "/" + this->name() + "_P2PV";
}
//=============================================================================
void BestPVAlg::checkTable(const Particle2Vertex::Table* table,
                           const std::string& tableLoc) const
{
  
  if (0!=table) {
    if ( msgLevel(MSG::VERBOSE) ) {
      verbose() << "Storing relations table in " 
                << tableLoc << endmsg;
      verbose() << "Table size " << table->relations().size() << endmsg;
    }
  } else {
    Error("No table created!",10).ignore();
  }
}

//=============================================================================
//  Finalize
//=============================================================================
StatusCode BestPVAlg::finalize() {

  if ( msgLevel(MSG::DEBUG) ) debug() << "==> Finalize" << endmsg;

  return GaudiAlgorithm::finalize();  // must be called after all other actions
}
//=============================================================================
