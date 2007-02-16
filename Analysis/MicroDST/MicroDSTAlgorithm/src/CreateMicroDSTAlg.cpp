// $Id: CreateMicroDSTAlg.cpp,v 1.2 2007-02-16 18:00:27 ukerzel Exp $
// Include files 

// from Gaudi
#include "GaudiKernel/DeclareFactoryEntries.h" 

// event
#include "Event/ODIN.h"

// local
#include "CreateMicroDSTAlg.h"

//-----------------------------------------------------------------------------
// Implementation file for class : CreateMicroDSTAlg
//
// 2007-02-09 : Ulrich Kerzel
//-----------------------------------------------------------------------------

// Declaration of the Algorithm Factory
DECLARE_ALGORITHM_FACTORY( CreateMicroDSTAlg );


//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
CreateMicroDSTAlg::CreateMicroDSTAlg( const std::string& name,
                                      ISvcLocator* pSvcLocator)
  : DVAlgorithm          ( name , pSvcLocator               ),
    m_OutputPrefix       ( "microDST"                       )
{
 
  declareProperty( "OutputPrefix"        , m_OutputPrefix        = "microDST" );
 
} //constructor
//=============================================================================
// Destructor
//=============================================================================
CreateMicroDSTAlg::~CreateMicroDSTAlg() {} 

//=============================================================================
// Initialization
//=============================================================================
StatusCode CreateMicroDSTAlg::initialize() {
  StatusCode sc = GaudiAlgorithm::initialize(); // must be executed first
  if ( sc.isFailure() ) return sc;  // error printed already by GaudiAlgorithm

  debug() << "==> Initialize" << endmsg;

  info() << "output prefix for microDST           " << m_OutputPrefix        << endmsg;

  return StatusCode::SUCCESS;
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode CreateMicroDSTAlg::execute() {

  debug() << "==> Execute" << endmsg;

  StatusCode sc;

  //
  // setups
  //
  m_ContainerMap.clear();  //reset

  //
  // setup containers holding elements to be written to microDST
  //
  verbose() << "setup the containers holding the quantities to be stored" << endmsg;

  //
  // store "official" primary vertices 
  //
  sc = CreateMicroDSTAlg::StorePV(LHCb::RecVertexLocation::Primary);
  if (sc != StatusCode::SUCCESS) {
    Warning("storing primary vertices into microDST failed", StatusCode::SUCCESS);
  }//if

  //
  // store ODIN bank to have acces to event time, run/event number etc
  //
  sc = CreateMicroDSTAlg::StoreOdin();
  if (sc != StatusCode::SUCCESS) {
    Warning("storing of ODIN bank failed", StatusCode::SUCCESS);
  }// if sc

  //
  // loop over all input locations and store the particles found there
  //
  
  LHCb::Particle::ConstVector particles = desktop()->particles();
  debug() << "got #particles  " << particles.size() << endmsg;
  if ( particles.size()<= 0) {
    verbose() << "no particle found, skip" << endmsg;
    return StatusCode::SUCCESS;
  }//if nBsCand

  LHCb::Particle::ConstVector::const_iterator iParticle;
  LHCb::Particle::ConstVector::const_iterator iParticleBegin = particles.begin();
  LHCb::Particle::ConstVector::const_iterator iParticleEnd   = particles.end();
  std::map<std::string, bool> locations;

  for (iParticle = iParticleBegin; iParticle != iParticleEnd; iParticle++){

    std::string locTES = CreateMicroDSTAlg::objectLocation((*iParticle)->parent());
    verbose() << "store particles from " << locTES << endmsg;

    //
    // get base location (w/o trailing "/Particles", etc)
    //
    std::string            tmpString  = "/Event/";
    std::string            tmp2String = "/Particles";

    std::string::size_type loc = locTES.find(tmpString);
    if ( loc != std::string::npos) {
      locTES.replace(loc, tmpString.length(),"");
      verbose() << "TES location ID is now " << locTES << endmsg;
    } //if loc

    loc  = locTES.find(tmp2String);
    if ( loc != std::string::npos) {
      locTES.replace(loc, tmp2String.length(),"");
      verbose() << "TES location ID is now " << locTES << endmsg;
    } //if loc

    locTES.insert(0, tmpString);
    verbose() << "base-location on TES " << locTES << endmsg;

    //
    // store user primary vertices
    //    
    // determine if this locaton has already been processed
    // (need to do only once per location)
    //
    std::map<std::string, bool>::const_iterator iLoc;
    iLoc = locations.find(locTES);
    if (iLoc != locations.end() ) {
      verbose() << "user PVs from this location already stored, total number of locations so far " 
                << locations.size()
                << endmsg;
    } else {
      verbose() << "store user primary vertices for this location" << endmsg;      
      sc = CreateMicroDSTAlg::StorePV(locTES+"/PrimaryVertices");
      if (sc != StatusCode::SUCCESS) {
        Warning("storing primary vertices into microDST failed");
      }//if
      locations[locTES] = true;  // mark this location done
    }// if iLoc

    //
    // now store particle
    //
    verbose() << "now call StoreParticle" << endmsg;
    sc = CreateMicroDSTAlg::StoreParticle(*iParticle); 
    if (sc != StatusCode::SUCCESS) {
      Warning("something went wrong when storing particle", StatusCode::SUCCESS);
    }// if sc
    
    //
    // now store links to PVs
    //
    verbose() << "now call StoreLink2PV" << endmsg;
    sc = CreateMicroDSTAlg::StoreLink2PV(*iParticle);
    if (sc != StatusCode::SUCCESS) {
      Warning("something went wrong when storing links to PV", StatusCode::SUCCESS);
    } // if sc

  } // for iParticle


  setFilterPassed(true);   // accept event
  return StatusCode::SUCCESS;
}

//=============================================================================
//  Finalize
//=============================================================================
StatusCode CreateMicroDSTAlg::finalize() {

  debug() << "==> Finalize" << endmsg;

  return GaudiAlgorithm::finalize();  // must be called after all other actions
}

//=============================================================================
StatusCode CreateMicroDSTAlg::StorePV(std::string location) {

  debug() << "now store primary vertices " << endmsg;


  // get primary vertices to store from TES
  LHCb::RecVertices *primaryVertices = NULL;
  if (exist<LHCb::RecVertices>(location)) {
    primaryVertices = get<LHCb::RecVertices>(location);
  } // if exist

  if (!primaryVertices) {
    Warning("Could not get input primary vertices",  StatusCode::SUCCESS);
    return StatusCode::SUCCESS;
  }
  verbose() << "got #input PV " << primaryVertices->size() << endmsg;

  // get container to store primary vertices in microDST
  LHCb::RecVertices *pvContainer = CreateMicroDSTAlg::getContainer<LHCb::RecVertices>(location);
  if (!pvContainer) {
    Warning("Could not get PV container", StatusCode::SUCCESS);
    return StatusCode::SUCCESS;
  } // if

  LHCb::RecVertices::const_iterator iPV;
  LHCb::RecVertices::const_iterator iPVBegin = primaryVertices->begin();
  LHCb::RecVertices::const_iterator iPVEnd   = primaryVertices->end();

  SmartRefVector< LHCb::Track >::const_iterator iTrack;
  SmartRefVector< LHCb::Track >::const_iterator iTrackBegin;
  SmartRefVector< LHCb::Track >::const_iterator iTrackEnd;  

  for (iPV = iPVBegin;  iPV !=iPVEnd ; iPV++){      
    verbose() << "consider next PV at position  " << (*iPV)->position() << endmsg;

    LHCb::RecVertex *pvClone = (*iPV)->clone();

    const SmartRefVector< LHCb::Track > &tracks = (*iPV)->tracks();
    iTrackBegin = tracks.begin();
    iTrackEnd   = tracks.end();  
    pvClone->clearTracks();
    for (iTrack = iTrackBegin; iTrack != iTrackEnd; iTrack++) {
      pvClone->addToTracks(*iTrack);
    }//for iTrack

    pvContainer->insert(pvClone,(*iPV)->key());  // use same key as original PV
  } // for PVIter

  return StatusCode::SUCCESS;

} //sc StorePV

//=============================================================================
StatusCode CreateMicroDSTAlg::StoreParticle(const LHCb::Particle * particle) {

   
  verbose() << "StoreParticle for particle with key " << particle->key()
            << " PID " << particle->particleID().pid()
            << "   m " << particle->measuredMass()/Gaudi::Units::GeV
            << endmsg;

  StatusCode sc;
  
  std::string locTES; // location on TES

  //
  // clone and store particle
  //
  verbose() << "store particle " << endmsg;
  locTES = CreateMicroDSTAlg::objectLocation(particle->parent());
  LHCb::Particles *particleContainer = CreateMicroDSTAlg::getContainer<LHCb::Particles>(locTES);
  if (!particleContainer) {
    Warning("Could not obtain particle container", StatusCode::SUCCESS);
    return StatusCode::SUCCESS;
  } // if !particleContainer    

  LHCb::Particle  *particleClone = particleContainer->object(particle->key());
  if (particleClone) {
    verbose() << "particle already in container" << endmsg;
  } else {
    verbose() << "particle not yet in container" << endmsg;
    particleClone = particle->clone();
    particleContainer->insert(particleClone,particle->key() );
  }// if particleClone


  //
  // check for endVertex and store
  //
  LHCb::Vertex *endVertexClone = NULL;
  if (particle->endVertex()) {
    verbose() << "now consider endVertex of particle" << endmsg;

    locTES = CreateMicroDSTAlg::objectLocation(particle->endVertex()->parent());
    verbose() << "location of endVertes on TES " << locTES << endmsg;
    LHCb::Vertices* vertexContainer = CreateMicroDSTAlg::getContainer<LHCb::Vertices>(locTES);
    if (vertexContainer) {
      LHCb::Vertex *endVertexClone = vertexContainer->object(particle->endVertex()->key());
      if (endVertexClone) {

        verbose() << "end vertex clone already exists in TES store" << endmsg;

      } else {

        verbose() << "clone and store endVertex " << endmsg;
        endVertexClone = particle->endVertex()->clone();
        vertexContainer->insert(endVertexClone, particle->endVertex()->key());
        endVertexClone->clearOutgoingParticles();  // remove SmartRefs and put in links to daughters

      } // if vertexContainer

      particleClone->setEndVertex(endVertexClone);

    } else {
      Warning("Vertex container could not be obtained ");
    } // if vertexContainer


    //
    // check for daugthers and store
    //
    
    // remove all daughters from clone before re-adding them on different DST location
    particleClone->clearDaughters();
    const SmartRefVector< LHCb::Particle > &daughters = particle->endVertex()->outgoingParticles();
    verbose() << "number of daughters " << daughters.size() << endmsg;
    
    SmartRefVector< LHCb::Particle >::const_iterator iDaughter;
    SmartRefVector< LHCb::Particle >::const_iterator iDaughterBegin = daughters.begin();
    SmartRefVector< LHCb::Particle >::const_iterator iDaughterEnd   = daughters.end();
    for (iDaughter = iDaughterBegin; iDaughter != iDaughterEnd; iDaughter++) {
      locTES = CreateMicroDSTAlg::objectLocation((*iDaughter)->parent());
      verbose() << " consider next daughter particle with PID " << (*iDaughter)->particleID().pid() 
                << " and key " << (*iDaughter)->key() 
                << " and TES location " << locTES
                << endmsg;

      LHCb::Particle *daughterClone = NULL;
      LHCb::Particles* daughterContainer = CreateMicroDSTAlg::getContainer<LHCb::Particles>(locTES);
      if (daughterContainer) {
         daughterClone = daughterContainer->object((*iDaughter)->key());
        if (daughterClone) {

          verbose() << "daughter already exists in TES" << endmsg;

        } else {
      
          verbose() << "clone and store daughter" << endmsg;
          daughterClone =  (*iDaughter)->clone();
          daughterContainer->insert(daughterClone,(*iDaughter)->key() ); // store to container
        } // if daughterClone

        particleClone->addToDaughters(daughterClone);  //set relations
        if (endVertexClone) {
          endVertexClone->addToOutgoingParticles(daughterClone);
        }// if endVertex
        
      } else {
        Warning("Daugher container could not be obtained");
      }
      
      // recursively store all daughter's daughters
      sc = CreateMicroDSTAlg::StoreParticle((*iDaughter));
                                            
      if (sc != StatusCode::SUCCESS) {
        Warning("something went wrong while storing the daughter particle");
      } // if sc
    } //for iDaughter
  }// if endVertex

  //
  // check for proto-particle and store
  //
  if (particle->proto()) {
    locTES= CreateMicroDSTAlg::objectLocation(particle->proto()->parent());
    verbose() << "consider new proto-particle with key "  << particle->proto()->key()
              << " on TES location " << locTES << endmsg;
    LHCb::ProtoParticle* protoClone = NULL;

    LHCb::ProtoParticles *protoPContainer = CreateMicroDSTAlg::getContainer<LHCb::ProtoParticles>(locTES);
    if (protoPContainer) {
      protoClone = protoPContainer->object(particle->proto()->key());
      if (protoClone) {

        verbose() << "proto-particle already exists in container" << endmsg;

      } else {
        
        verbose() << "store clone of proto-particle" << endmsg;
        protoClone = particle->proto()->clone();
        protoPContainer->insert(protoClone, particle->proto()->key());
      } // if protoClone

      particleClone->setProto(protoClone);

    } else {
      Warning("ProtoP container could not be obtained");
    }

    
     // set track
     if (particle->proto()->track()) {
       locTES = CreateMicroDSTAlg::objectLocation(particle->proto()->track()->parent());
       verbose() << "consider new track with key " << particle->proto()->track()->key()
                 << " on TES location " << locTES
                 << endmsg;

       LHCb::Track *trackClone = NULL;
       LHCb::Tracks *trackContainer =  CreateMicroDSTAlg::getContainer<LHCb::Tracks>(locTES);
       if (trackContainer) {
         trackClone = trackContainer->object(particle->proto()->track()->key());
         if (trackClone) {

           verbose() << "track already exists in container " << endmsg;

         } else {

           trackClone = particle->proto()->track()->clone();
           trackContainer->insert(trackClone, particle->proto()->track()->key());

         } // if trackClone

         protoClone->setTrack(trackClone);

       } else {
         Warning("Track container could not be obtained");
       }
     }// if track
     
     // set SmartRefs for Calo
     // -> doens't work yet
     
  }//if proto
  

  return StatusCode::SUCCESS;
}//sc StoreParticle

//=============================================================================
template<class T> T *CreateMicroDSTAlg::getContainer(std::string locTES){
  
  verbose() << "now get container for original TES location " << locTES << endmsg;

  // most orignial locations starts with /Event/<alg,Phys,MC, ..>/...
  //  - but not all, "/Event" is optional, some have it, some don't
  // insert the output-prefix to distinguish the microDST location
  // -> remove the "/Event" bit if found at beginning of routine
  std::string            tmpString = "/Event/";
  std::string::size_type loc       = locTES.find(tmpString);
  if ( loc != std::string::npos) {
    locTES.replace(loc, tmpString.length(),"");
    verbose() << "TES location ID is now " << locTES << endmsg;
  } //if loc
  

  std::map<std::string, ObjectContainerBase*>::const_iterator iMap;
  std::map<std::string, ObjectContainerBase*>::const_iterator iMapEnd = m_ContainerMap.end();

  // check if location already known
  iMap = m_ContainerMap.find(locTES);
  if (iMap != iMapEnd) {

    verbose() << "return existing container" << endmsg;
    return dynamic_cast<T*>(m_ContainerMap[locTES]);

  } else {

    verbose() << "container does not exisit yet, create" << endmsg;
    std::string containerLocation =  locTES;
    // now insert identifier for microDST after "/Event/"
    containerLocation.insert(0,"/Event/"+ m_OutputPrefix + "/");
    verbose() << "location of container in TES " << containerLocation << endmsg;

    // create new container and store into TES
    T* container = new T();
    put (container, containerLocation);     
    m_ContainerMap[locTES] = dynamic_cast<ObjectContainerBase*>(container);
    return dynamic_cast<T*>(m_ContainerMap[locTES]);

  }// if iMap

  return NULL; // something went wrong
}// getContainer


//=============================================================================
StatusCode CreateMicroDSTAlg::StoreLink2PV(const LHCb::Particle * particle) {

  //
  // save the connection between the particle and the related (primary) vertex
  //

  std::string linkerLocation = "/Event/" + m_OutputPrefix + "/" + LHCb::RecVertexLocation::Primary;

  verbose() << "save connection between (primary) vertex and particle to " 
            << linkerLocation << endmsg;
  verbose() << " particle PID " << particle->particleID().pid() 
            << " mass " << particle->measuredMass() << " +- " << particle->measuredMassErr()
            << endmsg;

  //
  // get particle clone from container in microDST part of TES
  //
  std::string locTES = CreateMicroDSTAlg::objectLocation(particle->parent());
  LHCb::Particles *particleContainer = CreateMicroDSTAlg::getContainer<LHCb::Particles>(locTES);
  if (!particleContainer) {
    Warning("Could not obtain particle container", StatusCode::SUCCESS);
    return StatusCode::SUCCESS;
  } // if !particleContainer    

  LHCb::Particle  *particleClone = particleContainer->object(particle->key());
  if (!particleClone) {
    Warning("Could not obtain particle clone in TES", StatusCode::SUCCESS);
    return StatusCode::SUCCESS;
  }// if !particleClone


  //
  // define linker
  //
  Part2Vertex* p2VLinker = new Part2Vertex(evtSvc(), msgSvc(), linkerLocation);

  //
  // get primary vertex related to original particle
  //

  const LHCb::VertexBase *relatedPV = desktop()->relatedVertex(particle);
  if (relatedPV) {
    const double weight = desktop()->weight(particle, relatedPV);
    verbose() << "particle with PID "    << particle->particleID().pid() 
              << " is related to PV at " << relatedPV->position() 
              << " #tracks "             << (dynamic_cast<const LHCb::RecVertex*>(relatedPV))->tracks().size()
              << " with weight "         << weight << endmsg;
    
    
    // find corresponding clone of primary vertex
    locTES = CreateMicroDSTAlg::objectLocation(relatedPV->parent());
    verbose() << "primary vertex comes from " << locTES << endmsg;
    LHCb::RecVertices *pvContainer = CreateMicroDSTAlg::getContainer<LHCb::RecVertices>(locTES);
    if (pvContainer) {
      verbose() << "got corresponding container" << endmsg;
      LHCb::RecVertex* pvClone = pvContainer->object(relatedPV->key()); //get corresponding cloned PV
      if (pvClone) {
        verbose() << "now link particle to clonedPV at " << pvClone->position()
                  << " with #tracks "                    << pvClone->tracks().size()
                  << " weight "                          << weight
                  << endmsg;
        // relate particle-clone to PV-clone with same weight
        // first  argument: source
        // second         : target
        p2VLinker->link(particleClone, pvClone, weight);

      } // if pvClone
    } // if pvContainer    
  } else { 
    verbose() << "particle not related to any PV " << endmsg;
  }//if relatedPV

  return StatusCode::SUCCESS;
} // sc StoreLink2PV


//=============================================================================
StatusCode CreateMicroDSTAlg::StoreOdin() {

  verbose() << "try to get ODIN bank" << endmsg;

  LHCb::ODIN * odin = NULL;
  if (exist<LHCb::ODIN>( LHCb::ODINLocation::Default ) ) {
    odin = get<LHCb::ODIN>( LHCb::ODINLocation::Default );
  } else {
    Warning("no ODIN bank found", StatusCode::SUCCESS);
    return StatusCode::SUCCESS;
  } // if exist
  
  verbose() << "now copy ODIN information" << endmsg;
  LHCb::ODIN *newOdin = new LHCb::ODIN();

  newOdin->setGpsTime          (odin->gpsTime());
  newOdin->setRunNumber        (odin->runNumber());
  newOdin->setEventType        (odin->eventType());
  newOdin->setOrbitNumber      (odin->orbitNumber());
  newOdin->setEventNumber      (odin->eventNumber());
  newOdin->setDetectorStatus   (odin->detectorStatus());
  newOdin->setErrorBits        (odin->errorBits());
  newOdin->setBunchId          (odin->bunchId());
  newOdin->setTriggerType      (odin->triggerType());
  newOdin->setReadoutType      (odin->readoutType());
  newOdin->setForceBit         (odin->forceBit());
  newOdin->setBunchCrossingType(odin->bunchCrossingType());
  newOdin->setBunchCurrent     (odin->bunchCurrent());
  
  put (newOdin, "/Event/"+ m_OutputPrefix + "/" + LHCb::ODINLocation::Default);

  verbose() << "ODIN values set to " 
            << " event time "        << newOdin->eventTime() 
            << " runNr "             << newOdin->runNumber ()
            << " orbit Nr "          << newOdin->orbitNumber()
            << " event type "        << newOdin->eventType()
            << " event number "      << newOdin->eventNumber()
            << " detector status "   << newOdin->detectorStatus () 
            << endmsg;

  return StatusCode::SUCCESS;
} // sc StoreOdin
//=============================================================================
