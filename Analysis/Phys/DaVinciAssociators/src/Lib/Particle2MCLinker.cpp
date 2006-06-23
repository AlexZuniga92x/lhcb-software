// $Id: Particle2MCLinker.cpp,v 1.1 2006-06-23 14:55:33 phicharp Exp $
// Include files 



// local
#include "DaVinciAssociators/Particle2MCLinker.h"
#include "Event/MCParticle.h"

using namespace LHCb;

//-----------------------------------------------------------------------------
// Implementation file for class : Particle2MCLink
//
// 2004-04-29 : Philippe Charpentier
//-----------------------------------------------------------------------------

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
//=============================================================================

StatusCode
Object2MCLinker::setAlgorithm( const int method, 
                             const std::vector<std::string>& containerList)
{
  return setAlgorithm(  Particle2MCMethod::algType[method], 
			Particle2MCMethod::extension[method], containerList);
}

StatusCode
Object2MCLinker::setAlgorithm( const std::string& algType,
                             const std::string& extension, 
                             const std::vector<std::string>& containerList)
{
  m_containerList = containerList;
  if( NULL != m_linkerAlg ) {
    if( algType != m_linkerAlgType) {
      // We change the algorithm on the fly... risky!
      m_linkerAlg = NULL;
    } else {
      return setAlgInputData( m_linkerAlg, m_containerList);
    }
  } 
  m_linkerAlgType = algType;
  m_extension     = extension;
  return StatusCode::SUCCESS;
}

StatusCode
Object2MCLinker::locateAlgorithm( const std::string& algType, 
                                const std::string& algName, 
                                IAlgorithm*& alg, 
                                const std::vector<std::string>& inputData)
{
  if( NULL != m_myGaudiAlg)
    m_myGaudiAlg->debug() << "==> Calling locateAlgorithm with type "
                          << algType << ", name " << algName <<endreq;
  else if( NULL != m_myGaudiTool)
    m_myGaudiTool->debug() << "==> Calling locateAlgorithm with type "
                          << algType << ", name " << algName <<endreq;
  
  if( NULL != alg ) return StatusCode::SUCCESS;
  if( algType.empty() ) return
    NULL != m_myGaudiAlg? m_myGaudiAlg->Error("No alg type given") : 
    NULL != m_myGaudiTool? m_myGaudiTool->Error("No alg type given"): 
    StatusCode::FAILURE;
  IAlgManager* algMgr = NULL;
  StatusCode sc = m_svcLoc->getService( "" , IAlgManager::interfaceID() , 
                                        (IInterface*&) algMgr );
  if( sc.isFailure() || NULL == algMgr) { return
    NULL != m_myGaudiAlg? m_myGaudiAlg->Error("Could not locate IAlgManager ", sc ) :
    NULL != m_myGaudiTool? m_myGaudiTool->Error("Could not locate IAlgManager ", sc ) :
    sc;}
  // check the existence of the algorithm
  typedef std::list<IAlgorithm*> Algs;
  Algs& algs = algMgr->getAlgorithms() ;
  for( Algs::iterator ia = algs.begin() ; algs.end() != ia ; ++ia )
  {
    if( 0 == *ia              ) { continue ; }
    if( (*ia)->name() != algName ) { continue ; }
    // algorithm is found ! 
    alg = *ia ;
    alg -> addRef() ;
    // Set its jobOptions
    return setAlgInputData( alg, inputData) ; 
  }
  // algorithm is noT found: try to create it! 
  sc = algMgr->createAlgorithm( algType , algName , alg );
  if( sc.isFailure()   ) { return
    NULL != m_myGaudiAlg? m_myGaudiAlg->Error("Could not create algorithm", sc ) :
    NULL != m_myGaudiTool? m_myGaudiTool->Error("Could not create algorithm", sc ) :
    sc;
  }
  // add the reference to the new algorithm 
  alg -> addRef() ;
  
  // Now initialise the algorithm
  sc = alg->sysInitialize() ;
  if( sc.isFailure() ) { 
    std::string err = "Error from algorithm " + algName + " initialization!";
    return 
      NULL != m_myGaudiAlg ? m_myGaudiAlg->Error( err, sc) :
      NULL != m_myGaudiTool ? m_myGaudiTool->Error( err, sc) :
      sc;
  }
  // Set jobOptions to the algorithm, supersede options file
  sc = setAlgInputData( alg, inputData);
  if( sc.isFailure() ) {
    std::string err = "Error setting InputData property to " + algName;
    return 
      NULL != m_myGaudiAlg ? m_myGaudiAlg->Error( err, sc) :
      NULL != m_myGaudiTool ? m_myGaudiTool->Error( err, sc) :
      sc;
  }
  //
  return StatusCode::SUCCESS ;
}

StatusCode
Object2MCLinker::setAlgInputData( IAlgorithm*& alg, 
                                  const std::vector<std::string>& inputData)
{
  StatusCode sc = StatusCode::SUCCESS;
  /* If requested, pass an InputData property to the algorithm */
  if( 0 != inputData.size() ) {
    IProperty* prop = dynamic_cast<IProperty*>( alg );
    
    if( prop ) {
      // Set OutputTable property to "" avoiding Relations table
      sc = prop->setProperty( "OutputTable", "");
      if( !sc.isSuccess() ) {
        std::string err = "Unable to set Property OutputTable in algo " + alg->name();
        return 
          NULL != m_myGaudiAlg ? m_myGaudiAlg->Error( err, sc) :
          NULL != m_myGaudiTool ? m_myGaudiTool->Error( err, sc) :
          sc;
      }
      std::string propString ;
      std::string sep = "\"";
      propString = "[";
      for( std::vector<std::string>::const_iterator inp = inputData.begin();
           inputData.end() != inp; inp++ ) {
        std::string inpStr = *inp;
        if( std::string::npos == inpStr.find( "/Particles") &&
             std::string::npos == inpStr.find( "/ProtoP") )
           inpStr += "/Particles";
        propString = propString + sep + inpStr ;
        sep = "\",\"";
      }
      propString = propString + "\"]";
      sc = prop->setProperty( "InputData", propString);
      if( sc.isSuccess() ) {
        std::string err = "Property InputData set to " + propString + " in algo " 
          + alg->name();
        if( NULL != m_myGaudiAlg ) m_myGaudiAlg->debug() << err;
        else if( NULL != m_myGaudiTool ) m_myGaudiTool->debug() << err;
      } else {
        std::string err = "Unable to set Property InputData in algo " + alg->name();
        return 
          NULL != m_myGaudiAlg ? m_myGaudiAlg->Error( err, sc) :
          NULL != m_myGaudiTool ? m_myGaudiTool->Error( err, sc) :
          sc;
      }
    } else {
      std::string err = "Unable to get IProperty pointer for algo " + alg->name();
      return 
        NULL != m_myGaudiAlg ? m_myGaudiAlg->Warning( err, sc) :
        NULL != m_myGaudiTool ? m_myGaudiTool->Warning ( err, sc) :
        sc;
    }
  }  
  return sc;
}

bool
Object2MCLinker::notFound( const std::string& contName)
{
  To test( m_evtSvc, NULL, contName + m_extension);
  return test.notFound();
}

bool
Object2MCLinker::notFound()
{
  for( std::vector<std::string>::const_iterator contIt = 
         m_containerList.begin();
       m_containerList.end() != contIt; contIt++) {
    if( !notFound(*contIt) ) return false;
  }
  return true;
}
  
void
Object2MCLinker::createLinks( const std::string& contName )
{
  // First find the contname is in the list
  if( m_containerList.end() == std::find( m_containerList.begin(), 
                                          m_containerList.end(), 
                                          contName) ) {
    // Container was not in the list... add it!
    m_containerList.push_back( contName );
    if( NULL != m_linkerAlg ) {
      std::string dbg = "    Add " + contName + " to InputData of "
        + m_linkerAlg->name();
      if( NULL != m_myGaudiAlg ) m_myGaudiAlg->debug() << dbg << endreq;
      else if( NULL != m_myGaudiTool ) m_myGaudiTool->debug() << dbg << endreq;
      StatusCode sc = setAlgInputData( m_linkerAlg, m_containerList);
      if( sc.isFailure() ) {
        std::string err = "Error adding " + contName + " to InputData of " 
          + m_linkerAlg->name();
        if( NULL != m_myGaudiAlg) m_myGaudiAlg->err() << err << endreq;
        else if( NULL != m_myGaudiTool) m_myGaudiTool->err() << err << endreq;
      }
    }
  }
  for( std::vector<std::string>::const_iterator contIt = 
         m_containerList.begin();
       m_containerList.end() != contIt; contIt++) {
    const std::string name = *contIt + m_extension;
    To test( m_evtSvc, NULL, name);
    if( test.notFound() && "" != m_linkerAlgType) {
      std::string dbg = "Link table for " + name + " not found, creating it";
      if( NULL != m_myGaudiAlg) m_myGaudiAlg->debug() << dbg << endreq;
      else if( NULL != m_myGaudiTool) m_myGaudiTool->debug() << dbg << endreq;
      if( NULL == m_linkerAlg ) {
        // Create the algorithm, as it has to be called
        StatusCode sc = locateAlgorithm( m_linkerAlgType, 
                                         m_name + "." + 
                                         m_linkerAlgType, 
                                         m_linkerAlg, 
                                         m_containerList);
        if( !sc.isSuccess() ) {
          std::string err = "Cannot locate algorithm of type " + m_linkerAlgType;
        if( NULL != m_myGaudiAlg) m_myGaudiAlg->err() << err << endreq;
        else if( NULL != m_myGaudiTool) m_myGaudiTool->err() << err << endreq;
        break;
        }
      }
      // Call the algorithm to get the table done
      if( NULL != m_myGaudiAlg)
        m_myGaudiAlg->debug() << "==> Executing Linker builder algorithm "
                              << m_linkerAlg->name() << endreq;
      else if( NULL != m_myGaudiTool)
        m_myGaudiTool->debug() << "==> Executing Linker builder algorithm "
                              << m_linkerAlg->name() << endreq;
      m_linkerAlg->sysExecute() ;
      test = To( m_evtSvc, NULL, name);
    }
    if( *contIt == contName ) m_linkTo = test;
  }
}

Object2MCLinker::To*
Object2MCLinker::getLink( const std::string& contName )
{
  // If the lists are empty, create them
  std::string name = contName+ m_extension;
  To test( m_evtSvc, NULL, name);
  if( test.notFound() ) {
    createLinks( contName );
  } else {
    m_linkTo = test;
  }
  return &m_linkTo;  
}

int
Object2MCLinker::associatedMCP( const KeyedObject<int>* obj)
{
  int n = 0;
  for( const LHCb::MCParticle* mcPart = firstMCP(obj); 
       NULL != mcPart; 
       mcPart = nextMCP(), n++);
  return n;
}

const LHCb::MCParticle* 
Object2MCLinker::firstMCP( const KeyedObject<int>* part, double& weight)
{
  const LHCb::MCParticle* mcPart = firstMCP(part);
  weight = NULL != mcPart ? weightMCP() : 0.;
  return mcPart;
}


const LHCb::MCParticle* 
Object2MCLinker::firstMCP( const KeyedObject<int>* part)
{
  if( NULL == m_myGaudiAlg && NULL == m_myGaudiTool ) return NULL;

  std::string contName = 
    containerName(dynamic_cast<const ContainedObject*>(part)) ;
  if( "/Event/" == contName.substr(0,7) ) {
    contName = contName.substr(7);
  }
  if("" == contName) {
    m_linkTo = To( m_evtSvc, NULL, "");
    return NULL;
  }
  To* link =  getLink( contName );
  
  return NULL != link ? link->first(part) : NULL;
}


// Helper methods to create a LinkerWithKey table if needed
Object2MCLinker::Linker* 
Object2MCLinker::linkerTable( const std::string& name)
{
  if( NULL == m_myGaudiAlg && NULL == m_myGaudiTool ) return NULL;

  To test(m_evtSvc,NULL, name);
  return linkerTable( name, test);
}


Object2MCLinker::Linker* 
Object2MCLinker::linkerTable( const std::string& name, To& test)
{
  if( NULL == m_myGaudiAlg && NULL == m_myGaudiTool ) return NULL;

  if( test.notFound() ) {
    m_linkerTable = Object2MCLinker::Linker( m_evtSvc, 
                                           m_msgSvc, 
                                           name);
    if( NULL != m_myGaudiAlg) {
      m_myGaudiAlg->debug() << "Linker table " << name << " created" <<endreq;
    }
    else if( NULL != m_myGaudiTool) {
      m_myGaudiTool->debug() << "Linker table " << name << " created" <<endreq;
    }
    test = To( m_evtSvc, NULL, name);
    return &m_linkerTable;
  } else {
    if( NULL != m_myGaudiAlg)
      m_myGaudiAlg->debug() << "Linker table " << name << " found" <<endreq;
    if( NULL != m_myGaudiTool)
      m_myGaudiTool->debug() << "Linker table " << name << " found" <<endreq;
  }
  return NULL;
}

bool
Object2MCLinker::checkAssociation( const KeyedObject<int>* obj,
                                 const MCParticle* mcPart)
{
  const MCParticle* mcp = firstMCP(obj);
  while( NULL != mcp ) {
    if( mcp == mcPart ) return true;
    mcp = nextMCP();
  }
  return false;
}
