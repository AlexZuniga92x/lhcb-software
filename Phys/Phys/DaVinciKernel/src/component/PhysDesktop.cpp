// $Id: PhysDesktop.cpp,v 1.13 2005-02-09 07:32:54 pkoppenb Exp $
// Include files 

// from Gaudi
#include "GaudiKernel/ToolFactory.h"
#include "GaudiKernel/GaudiException.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/IAlgorithm.h"

// data
#include "Event/EventHeader.h"
#include "Event/Vertex.h"
#include "Event/Particle.h"

// local
#include "PhysDesktop.h"
#include "Kernel/IParticleMaker.h"
#include "Kernel/IPVLocator.h"

/*-----------------------------------------------------------------------------
 * Implementation file for class : PhysDesktop base class 
 *
 * 18/02/2002 : Sandra Amato
 * 04/03/2004 : Hugo Ruiz : automatized outputLocation = algorithm name
 * 11/08/2004 : Patrick Koppenburg : Make it a GaudiTool
 * 17/12/2004 : Patrick Koppenburg : Add PVLocator tool
 * 08/02/2005 : Patrick Koppenburg : Split vertices into primaries and secondaries
 *-----------------------------------------------------------------------------
 */

// Declaration of the Tool Factory
static const  ToolFactory<PhysDesktop>           s_factory ;
const        IToolFactory& PhysDesktopFactory = s_factory ; 

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
PhysDesktop::PhysDesktop( const std::string& type,
                          const std::string& name,
                          const IInterface* parent )
  : GaudiTool ( type, name , parent )
  , m_pMaker(0)
  , m_locationWarned(false)
  , m_PVLocator(0){
  
  // Declaring implemented interfaces
  declareInterface<IPhysDesktop>(this);
  declareInterface<IIncidentListener>(this);
  
  // Declare properties
  //                    loading conditions
  
  // Type of input particles (Ax, MC, Proto) maker.
  declareProperty( "ParticleMakerType",m_pMakerType="" );
  
  //                    input & output locations
  declareProperty( "InputPrimaryVertices", m_primVtxLocn = "" );
  m_inputLocn.clear();
  declareProperty( "InputLocations", m_inputLocn );

  //  REMOVED TO KILL COMPATIBILITY WITH OLD INPUT FILES
  //  declareProperty( "OutputLocation", m_outputLocn = "/Event/Phys/User");
  
  const IInterface* itemp = this->parent();
  const IAlgorithm* ialg = dynamic_cast<const IAlgorithm*>(itemp);
  if( !ialg ) {
    StatusCode scExc;
    throw GaudiException("Parent of PhysDesktop is not an algorithm",
                         this->name(),scExc);
  }
  
  m_outputLocn = "/Event/Phys/"+ialg->name();
  
}

//=============================================================================
// Initialize method, retrieve necessary services 
//=============================================================================
StatusCode PhysDesktop::initialize() {
  
  StatusCode sc = GaudiTool::initialize() ;
  if (!sc) return sc ;
  debug() << ">>>   PhysDesktop::initialize() " << endmsg;

  // Register to the Incident service to be notified at the end of one event
  IIncidentSvc* incsvc=0;

  sc = service("IncidentSvc", incsvc, true);
  if( sc.isSuccess() ) {
    incsvc->addListener(this,"EndEvent",100);
  }
  else {
    fatal() << " Unable to locate Incident Service" << endmsg;
    return sc;
  }
  
  if ( m_pMakerType == "" ){
    info() << " No ParticleMaker requested in job options" 
           << endmsg;
    info() << " Only previously produced particles will be loaded"
           << endmsg;
  }
  else{
    
    // Retrieve the ParticleMaker tool:
    info() << " Using " << m_pMakerType << " to make particles"
           << endmsg;
    m_pMaker = tool<IParticleMaker>(m_pMakerType, this);
    if(!m_pMaker) {
      fatal() << " Unable to retrieve " << m_pMakerType << endmsg;
      return sc;
    }
  }
  
  // Check if InputLocation has been set
  if( m_inputLocn.size() == 0 ) {
    info() << "No Input from previous processing requested"
           << endmsg;
  }
  else {
    info() << "Particles and Vertices will be loaded from "
           << std::endl ;
    for( std::vector<std::string>::iterator iloc = m_inputLocn.begin(); 
         iloc != m_inputLocn.end(); iloc++ ) {
      
      info() << "   ==>" << *iloc << std::endl;
    }
    info() << endmsg;
  }
  
  // PVLocator tool
  m_PVLocator = tool<IPVLocator>("PVLocator");
  if( !m_PVLocator ){
    err() << " Unable to retrieve PV Locator tool" << endmsg;
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
  
}

//=============================================================================
// Implementation of Listener interface 
//=============================================================================
void PhysDesktop::handle(const Incident&) 
{
  cleanDesktop();
}

//=============================================================================
// Provides a reference to its internal container of particles
//=============================================================================
const ParticleVector& PhysDesktop::particles()
{
  return m_parts;
}

//=============================================================================
// Provides a reference to its internal container of vertices
//=============================================================================
const VertexVector& PhysDesktop::vertices()
{
  
  if ( ( m_verts.empty()) && !((m_primVerts.empty()) && (m_secVerts.empty()) )){
    Warning("Deprecated method: Use faster primaryVertices() and secondaryVertices() methods instead");
    m_verts = m_primVerts ;
    m_verts.insert(m_verts.end(),m_secVerts.begin(),m_secVerts.end()); // concatenate
  }
  
  return m_verts;
}

//=============================================================================
// Provides a reference to its internal container of vertices
//=============================================================================
const VertexVector& PhysDesktop::primaryVertices()
{
  return m_primVerts;
}

//=============================================================================
// Provides a reference to its internal container of vertices
//=============================================================================
const VertexVector& PhysDesktop::secondaryVertices()
{
  return m_secVerts;
}

//============================================================================
// Clean local data, this is called by the handle method and at finalization
//============================================================================
StatusCode PhysDesktop::cleanDesktop() 
{
  
  verbose() << "cleanDesktop()" << endmsg;
  verbose() << "Removing all particles from desktop" << endmsg;
  
  // Some particle have been saved to the TES, so they belong to it
  // others do not and need to be deleted by the PhysDesktop
  verbose() << "Number of particles before cleaning = "
            << m_parts.size() << endmsg;
  
  int iTEScount = 0;
  while ( m_parts.size() > 0 ) {
    Particle* ipart = m_parts.back();
    m_parts.pop_back();
    // Particles in KeyedContainers (=>TES) have parent 
    if( ipart->parent() ) {
      iTEScount++;
    }
    else {
      delete ipart;
    }
  }
  
  verbose() << "Particle in TES = " << iTEScount << endmsg;
  
  //m_parts.clear();
  
  verbose() << "Removing all vertices from desktop" << endmsg;
  verbose() << "Number of vertices before cleaning = "
            << m_parts.size() << endmsg;
  
  iTEScount = 0;
  while ( m_secVerts.size() > 0 ) {
    Vertex* ivert = m_secVerts.back();
    m_secVerts.pop_back();
    if( ivert->parent() ) {
      iTEScount++;
    }
    else {
      delete ivert;
    }
  }
  while ( m_primVerts.size() > 0 ) {
    Vertex* ivert = m_primVerts.back();
    m_primVerts.pop_back();
    if( ivert->parent() ) {
      iTEScount++;
    }
    else {
      delete ivert;
    }
  }
  
  return StatusCode::SUCCESS;
  
}

//=============================================================================
// Finalize 
//=============================================================================
StatusCode PhysDesktop::finalize()
{
  cleanDesktop();
  if( m_pMaker ) toolSvc()->releaseTool( m_pMaker );  
  return GaudiTool::finalize() ;
}
//=============================================================================
// Create a new particle in the DeskTop
//=============================================================================
Particle* PhysDesktop::createParticle( Particle* partToSave ){
  
  verbose() << "createParticle in desktop" << endmsg;
  
  // Input particle is given check if it already exist in the stack
  if( ( 0 != partToSave ) && ( 0 != partToSave->desktop() ) ) {
    verbose() << "Input particle momentum = " 
              << partToSave->momentum().px() << " ," 
              << partToSave->momentum().py() << " ," 
              << partToSave->momentum().pz() << " parent " << partToSave->parent() << endmsg;
    verbose() << "Particle address " << partToSave << endmsg;
    return partToSave;
  }
  
  // Create new particle on the heap
  Particle* saveP = new Particle();
  verbose() << "New particle momentum = " 
            << saveP->momentum().px() << " ," 
            << saveP->momentum().py() << " ," 
            << saveP->momentum().pz() << " parent " << partToSave->parent() << endmsg;
  
  // Input Particle from stack is given as input to fill newly created particle
  if( ( 0 != partToSave) && ( 0 == partToSave->desktop() ) ) {
    // Copy contents to newly created particle
    Particle& savePcont = *saveP;
    savePcont = *partToSave;
    verbose() << "Input particle momentum = " 
              << saveP->momentum().px() << " ," 
              << saveP->momentum().py() << " ," 
              << saveP->momentum().pz() << endmsg;
    // Check if link to endProducts exist and set it
    if( 0 != partToSave->endVertex() ) {
      Vertex* saveV = createVertex( partToSave->endVertex() );
      saveP->setEndVertex(saveV);
    }
    // Link to outgoing particles is followed through the createVertex
    // Link to originators will be correct because they are in the heap
    // so their pointer is valid
  }
  
  // Put in the desktop container
  saveP->setDesktop(1);
  verbose() << "Momentum of new particle in desktop = "
            << saveP->momentum().px() << " ," 
            << saveP->momentum().py() << " ," 
            << saveP->momentum().pz() << " parent " << partToSave->parent() << endmsg;
  verbose() << "Address of new particle in desktop = "
            << saveP << endmsg;
  m_parts.push_back(saveP);     
  return saveP;
  
}
//=============================================================================
// Create a new vertex
//=============================================================================
Vertex* PhysDesktop::createVertex( Vertex* vtxToSave ){

  verbose() << "createVertex in desktop" << endmsg;
  
  // Input vertex is given check if it already exist in the stack
  if( ( 0 != vtxToSave ) && ( 0 != vtxToSave->desktop() ) ) {
    verbose() << "Input vertex position = " 
              << vtxToSave->position().x() << " ," 
              << vtxToSave->position().y() << " ," 
              << vtxToSave->position().z() << endmsg;
    verbose() << "Vertex address " << vtxToSave << endmsg;
    return vtxToSave;
  }
  
  // Create new vertex on the heap
  Vertex* saveV = new Vertex();
  verbose() << "New vertex position = " 
            << saveV->position().x() << " ," 
            << saveV->position().y() << " ," 
            << saveV->position().z() << endmsg;
  
  // Input vertex from stack is given as input to fill new created vertex
  if( ( 0 != vtxToSave ) && ( 0 == vtxToSave->desktop() ) ) {
    verbose() << "Input vertex position = " 
              << saveV->position().x() << " ," 
              << saveV->position().y() << " ," 
              << saveV->position().z() << endmsg;
    // Copy contents to newly created vertex
    Vertex& saveVcont = *saveV;
    saveVcont = *vtxToSave;
    saveV->clearProducts();
    // Check if link to endProducts exist and set it
    SmartRefVector<Particle> outP = vtxToSave->products();
    SmartRefVector<Particle>::iterator ip;
    for( ip = outP.begin(); ip != outP.end(); ip++ ) {
      Particle* saveP = createParticle( *ip );
      saveV->addToProducts(saveP);
    }
  }
  
  // Put in the desktop container
  saveV->setDesktop(1);
  verbose() << "Position of new vertex in desktop = "
            << saveV->position().x() << " ," 
            << saveV->position().y() << " ," 
            << saveV->position().z() << endmsg;
  verbose() << "Address of new vertex in desktop = "
            << saveV << endmsg;
  m_secVerts.push_back(saveV);
  return saveV;
  
}


//=============================================================================
// Save all particles & vertices in the Desktop to the TES
//=============================================================================
StatusCode PhysDesktop::saveDesktop() {
  
  verbose() << "Save all new particles and vertices in desktop " << endmsg; 
  return saveDesktop( m_parts, m_secVerts );
  
}

//=============================================================================
// Save all specified particles & vertices in the Desktop to the TES
//=============================================================================
StatusCode PhysDesktop::saveDesktop( ParticleVector& pToSave, 
                                     VertexVector& vToSave) {
  
  verbose() << "Save specified particles and vertices " << endmsg;
  
  // Register the particles containers in the store
  Particles* particlesToSave = new Particles();
  
  std::string location = m_outputLocn+"/Particles";
  
  for( ParticleVector::const_iterator icand = pToSave.begin();
       icand != pToSave.end(); icand++ ) {
    // Check if this was already in a Gaudi container (hence in TES)
    if(  0 == (*icand)->parent() ) {
      particlesToSave->insert(*icand);
      verbose() << "  Saving " << (*icand)->particleID().pid() 
                << " with P= " << (*icand)->momentum() << " m=" 
                << (*icand)->mass() << endmsg ;
    } else {
      verbose() << "Skipping " << (*icand)->particleID().pid() 
                << " with P= " << (*icand)->momentum() << " m=" 
                << (*icand)->mass() << " with parent " << (*icand)->parent() << endmsg ;
    }
  }

  verbose() << "Saving " << particlesToSave->size()
            << " new particles in " << location << " from " << pToSave.size() 
            << " total particles in desktop " << endmsg;
 
  StatusCode sc = put(particlesToSave,location);
  if ( sc.isFailure() ) {
    delete particlesToSave;
    err() << "    Unable to register" << location << endmsg;
    return StatusCode::FAILURE;
  }
  //else {
  //  verbose() << "Removing particles saved to TES from desktop" 
  //      << endmsg;
  //  pToSave.clear();
  //}
  
  // Register the vertices in the store
  Vertices* verticesToSave = new Vertices();
  for( VertexVector::iterator iver = vToSave.begin();
       iver != vToSave.end(); iver++ ) {
    // Check if this was already in a Gaudi container (hence in TES)
    if( 0 == (*iver)->parent() ) {
      verticesToSave->insert(*iver);
    }
  }
  
  location = m_outputLocn+"/Vertices";
  
  verbose() << "Saving " << verticesToSave->size()
            << " new vertices in " << location << " from " << vToSave.size() 
            << " vertices in desktop " << endmsg;
  
  sc = put(verticesToSave,location);
  
  if ( !sc ) {
    delete verticesToSave;
    err() << "    Unable to register" << location << endmsg;
    return StatusCode::FAILURE;
  }
  //else {
  //  verbose() << "Removing vertices saved to TES from desktop" 
  //      << endmsg;
  //  vToSave.clear();
  //}
  
  return StatusCode::SUCCESS;
  
}


//=============================================================================
// Save only this list of particles and their tree in TES
// If bottom of trees is in TES it will be copied in the new location
//=============================================================================
StatusCode PhysDesktop::saveTrees(ParticleVector& pToSave) {
  
  verbose() << " PhysDesktop SaveTrees(ParticleVector)" << endmsg;
  
  // Find all particles that will need to be saved and put them in
  // a container
  ParticleVector allpToSave;
  VertexVector allvToSave;
  for( ParticleVector::iterator icand = pToSave.begin();
       icand != pToSave.end(); icand++ ) {
    // Find all descendendant from this particle
    verbose() << "  Getting " << (*icand)->particleID().pid() 
              << " with P= " << (*icand)->momentum() << " parent " 
              << (*icand)->parent() << endmsg ;
    findAllTree( *icand, allpToSave, allvToSave);
  }
  
  return saveDesktop( allpToSave, allvToSave );
  
}

//=============================================================================
// Save only this list of vertices and their trees in TES
//=============================================================================
StatusCode PhysDesktop::saveTrees(VertexVector& vToSave){
  
  verbose() << " PhysDesktop SaveTrees(VertexVector) " << endmsg;
  
  // Find all particles that will need to be saved and put them in
  // a container
  ParticleVector allpToSave;
  VertexVector allvToSave;
  for( VertexVector::iterator ivert = vToSave.begin(); 
       ivert != vToSave.end(); ivert++ ) {
    // Find all descendendant from this vertex
    findAllTree( *ivert, allpToSave, allvToSave);
  }
  
  return saveDesktop( allpToSave, allvToSave );
  
}

//=============================================================================
// Save only particles in desktop corresponding to this code
//=============================================================================
StatusCode PhysDesktop::saveTrees( int partid ) {
  
  verbose() << "PhysDesktop saveParticles(pid code)" 
            << "type = " << partid << endmsg;
  
  ParticleVector pToSave;
  for( ParticleVector::iterator icand = m_parts.begin();
       icand != m_parts.end(); icand++ ) {
    if( ((*icand)->particleID().pid()) == partid ) {
      pToSave.push_back(*icand);
    }
  }
  //  if( pToSave.size() > 0 ) {
  return saveTrees( pToSave );
  //   }
  
  //  return StatusCode::SUCCESS;
  
}

//=============================================================================
// Clone selected particles
//=============================================================================
StatusCode PhysDesktop::cloneTrees( ParticleVector& pToSave ) {
  
  ParticleVector cloned;
  for (ParticleVector::iterator i=pToSave.begin();i!=pToSave.end();++i) {
    Particle *clone = (*i)->clone();
    cloned.push_back(clone);
    verbose() << "Clone " << clone->particleID().pid() << " with momentum " << 
      clone->momentum() << " m=" << clone->mass() << endmsg ;
  }
  return saveTrees(cloned);
    
}

//=============================================================================
// Find all particle and vertices connected to this tree
//=============================================================================
void PhysDesktop::findAllTree( Particle* part, ParticleVector& parts,
                               VertexVector& verts ) {
  
  parts.push_back( part );
  if( 0 == part->endVertex() ) {
    return;
  }
  //verts.push_back( part->endVertex() );
  findAllTree( part->endVertex(), parts, verts );
  return;
  
}

//=============================================================================
// Find all particle and vertices connected to this tree
//=============================================================================
void PhysDesktop::findAllTree( Vertex* vert, ParticleVector& parts,
                               VertexVector& verts ) {
  
  verts.push_back( vert );
  SmartRefVector<Particle> prod = vert->products();
  for( SmartRefVector<Particle>::iterator ip = prod.begin();
       ip != prod.end(); ip++ ) {
    //parts.push_back(*ip);
    //if( 0 != (*ip)->endVertex() ) {
    //  findAllTree( (*ip)->endVertex(), parts, verts );
    //}
    findAllTree( *ip, parts, verts );
  }
  return;
}

//=============================================================================
// Method retrieving the data from the store.
// Automatically called by DVAlgorithm::sysExecute()
//=============================================================================
StatusCode PhysDesktop::getEventInput(){

  verbose() << ">>> Hello from getEventInput " << endmsg;
  verbose() << "Initial size of local containers (P,PV,SV) = " << m_parts.size() 
            << ", " << m_primVerts.size() << ", " <<  m_secVerts.size()<< endmsg;
  
  // Make particles with particle maker
  if ( m_pMaker ) {
    StatusCode sc = makeParticles();
    if (!sc) return sc;
  } 
  if ( !m_verts.empty()) m_verts.clear(); // to make sure it is clean in this event

  // Retrieve Primary vertices
  if( "None" == m_primVtxLocn ) {
    verbose() << "Not loading any primary vertices" << endmsg;
  } else {    
    StatusCode sc = getPrimaryVertices();
    if (!sc) return sc;
  }

  // Retrieve Particles & Vertices from all previous processing
  // as specified in jobOptions
  if (!m_inputLocn.empty()) {
    StatusCode sc = getParticles();
    if (!sc) return sc;
  }
  
  return StatusCode::SUCCESS;
  
}

//=============================================================================
// Make Particles
//=============================================================================
StatusCode PhysDesktop::makeParticles(){
  // Make particles starting from MC or reconstruction objects
    
  verbose() << "PhysDesktop:Calling " << m_pMakerType 
            << "::makeParticles() " 
            << endmsg;
    
  // Remember that these particles belong to the Desktop and are not
  // in a TES container yet
  StatusCode sc = m_pMaker->makeParticles(m_parts);
  
  if (!sc) {
    verbose() << " not able to make particles " << endmsg;
    return sc;
  }
  
  verbose() << "Number of Particles from " << m_pMakerType
            << " are " << m_parts.size() << endmsg;
  
  // Flag these particles to be in PhysDesktop
  for( ParticleVector::iterator ip = m_parts.begin(); ip != m_parts.end(); ip++ ) {
    (*ip)->setDesktop(1);
  }
  return sc;
}
//=============================================================================
// Get Particles
//=============================================================================
StatusCode PhysDesktop::getParticles(){

  for( std::vector<std::string>::iterator iloc = m_inputLocn.begin(); 
       iloc != m_inputLocn.end(); iloc++ ) {

    // Retrieve the particles:
    std::string location = (*iloc)+"/Particles";
    
    bool foundpart = exist<Particles>( location ) ;
    if (!foundpart){
      Warning("No particles at location "+location);
    } else {
      Particles* parts = get<Particles>( location );
      if ( !parts ) { 
        verbose() << "Unable to retrieve Particles from " << location << endmsg;
      } else if ( parts->empty() ) {
        verbose() << "No Particles retrieved from " << location << endmsg;
      } else {
      
        // Msg number of Particles retrieved
        verbose() << "    Number of Particles retrieved from " 
                  << location << " = " << parts->size() << endmsg;
      
        for( Particles::iterator icand = parts->begin(); icand != parts->end(); icand++ ) {
          (*icand)->setDesktop(1);
          m_parts.push_back(*icand);
        }
      }    
      verbose() << "Number of Particles after adding " 
                << location << " = " << m_parts.size() << endmsg;
    }
    
    // Retrieve the vertices:
    location = (*iloc)+"/Vertices";
    
    if (!exist<Vertices>( location )){
      if (foundpart) Warning("No Vertices at location "+location);
    } else {      

      Vertices* verts = get<Vertices>( location );    
      
      if ( !verts ) { 
        verbose() << "Unable to retrieve vertices from " << location << endmsg;
      }
      else if( 0 == verts->size() ) {
        verbose() << "No vertices retrieved from " << location << endmsg;
      }      
      else {
        
        // Msg number of vertices retrieved
        verbose() << "    Number of vertices retrieved from "
                  << location << " = " << verts->size() << endmsg;
      
        for( Vertices::iterator ivert = verts->begin();
             ivert != verts->end(); ++ivert ) {
          (*ivert)->setDesktop(1);
          m_secVerts.push_back(*ivert);
        }
      }
      verbose() << "Number of vertices after adding " 
                << location << " = " << m_secVerts.size() << endmsg;
      
    }  
  }
  verbose() << "    Total number of particles " << m_parts.size() << endmsg;
  verbose() << "    Total number of secondary vertices " << m_secVerts.size() << endmsg; 
  
  return StatusCode::SUCCESS;
}
//=============================================================================
// Get PV
//=============================================================================
StatusCode PhysDesktop::getPrimaryVertices(){
  
  std::string primVtxLocn ;
  if ( m_primVtxLocn == "" ) primVtxLocn = m_PVLocator->getPVLocation() ;
  else primVtxLocn = m_primVtxLocn ;

  verbose() << "Getting PV from " << primVtxLocn << endmsg;

  if ( !exist<Vertices>( primVtxLocn )){
    Warning("Primary vertex location `"+primVtxLocn+"' does not exist") ;
    if ( m_primVtxLocn == "" ) Warning("This location is obtained from PVLocator tool");
    return StatusCode::SUCCESS; // no PV
  }
       
  Vertices* verts = get<Vertices>( primVtxLocn );
  if( ! verts ) {
    verbose() << " Unable to retrieve vertices from " << primVtxLocn << endmsg;
  } else if( verts->empty() ) {
    verbose() << " No vertices retrieved from  " << primVtxLocn << endmsg;
  } else { 
    verbose() << "    Number of primary vertices  = " << verts->size() << endmsg;
    
    for( Vertices::iterator ivert = verts->begin();
         ivert != verts->end(); ivert++ ) {
      verbose() << "    Vertex coordinates = ( " 
                << (*ivert)->position().x() 
                << " , " << (*ivert)->position().y() 
                << " , " << (*ivert)->position().z() << " ) " << endmsg;
      
      verbose() << "    Vertex ChiSquare = " << (*ivert)->chi2() 
                << endmsg;  
      // Put them in local containers
      (*ivert)->setDesktop(1);
      m_primVerts.push_back(*ivert);
    }
  }
  verbose() << "Number of Vertices from " << primVtxLocn << " are " << m_primVerts.size() << endmsg;
  return StatusCode::SUCCESS;
}
//=============================================================================
// Impose OutputLocation
//=============================================================================
void PhysDesktop::imposeOutputLocation(std::string outputLocationString){
  if (outputLocationString != m_outputLocn) {
    if (m_locationWarned) {
      warning() << "Non-standard output location imposed: "
                << outputLocationString << endmsg;
      m_locationWarned = true ;
    }
    m_outputLocn = outputLocationString;
  }
  return;
}
